/*
 *  isl29023_poll.c - Linux kernel modules for ambient light sensor
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/input.h>
#include <linux/wakelock.h>

#define ISL29023_DRV_NAME	"isl29023"
#define DRIVER_VERSION		"0.9"

/*
 * Defines
 */
#define ISL29023_COMMAND1		0x00
#define ISL29023_MODE_SHIFT		(5)
#define ISL29023_MODE_MASK		(0x7 << ISL29023_MODE_SHIFT)
#define ISL29023_INT_FLAG_SHIFT		(2)
#define ISL29023_INT_FLAG_MASK		(0x1 << ISL29023_INT_FLAG_SHIFT)
#define ISL29023_INT_PERSISTS_SHIFT	(0)
#define ISL29023_INT_PERSISTS_MASK	(0x3 << ISL29023_INT_PERSISTS_SHIFT)

#define ISL29023_COMMAND2		0x01
#define ISL29023_RES_SHIFT		(2)
#define ISL29023_RES_MASK		(0x3 << ISL29023_RES_SHIFT)
#define ISL29023_RANGE_SHIFT		(0)
#define ISL29023_RANGE_MASK		(0x3 << ISL29023_RANGE_SHIFT)

#define ISL29023_REG_LSB_SENSOR		0x02
#define ISL29023_REG_MSB_SENSOR		0x03
#define ISL29023_REG_IRQ_TH_LO_LSB	0x04
#define ISL29023_REG_IRQ_TH_LO_MSB	0x05
#define ISL29023_REG_IRQ_TH_HI_LSB	0x06
#define ISL29023_REG_IRQ_TH_HI_MSB	0x07

#define ISL29023_STANDARD_RANGE		0x18
#define ISL29023_EXTENDED_RANGE		0x1d

#define ISL29023_NUM_CACHABLE_REGS	8
#define DEF_RANGE			2


/* start time delay for light sensor in nano seconds */
#define LIGHT_SENSOR_START_TIME_DELAY 50000000

#define BUFFER_NUM	6

#define isl29023_dbgmsg(str, args...) pr_debug("%s: " str, __func__, ##args)

/*
 * Structs
 */

struct isl29023_data {
	struct i2c_client *client;
	struct input_dev *light_input_dev;
	struct mutex update_lock;
	struct workqueue_struct *wq;
	struct wake_lock prx_wake_lock;
	struct work_struct work_light;
	struct hrtimer timer;
	ktime_t light_poll_delay;
	int lux_value_buf[BUFFER_NUM];
	int index_count;
	bool buf_initialized;

	unsigned int power_state:1;
	unsigned int operating_mode:1;
	unsigned int enable:1;
};

/*
 * Global data
 */

static int isl29023_debug = 0;

static const u8 ISL29023_MODE_RANGE[2] = {
	ISL29023_STANDARD_RANGE, ISL29023_EXTENDED_RANGE,
};

/*
 * Management functions
 */

static int isl29023_set_power_state(struct i2c_client *client, int state)
{
	struct isl29023_data *data = i2c_get_clientdata(client);
	int ret;

	if (state == 0)
		ret = i2c_smbus_write_byte_data(client, ISL29023_COMMAND1, 0x0);
	else {
		ret = i2c_smbus_write_byte_data(client, ISL29023_COMMAND1, 0xa0);
	}

	data->power_state = state;

	return ret;
}

/*
 * LUX calculation
 */

/*
 * This function is described into Taos ISL29023 Designer's Notebook
 * pages 2, 3.
 */
static int isl29023_calculate_lux(u8 ch0, u8 ch1)
{
	unsigned int lux;

	u16 lux_data = ch1<<8;/*MSB*/
	lux_data += ch0;/*plus with lsb*/

	if(isl29023_debug)
		printk("%s:lux reading is 0x%x\n",__FUNCTION__,lux_data);
	/*
	The EQ of transformation is:
	Ecal = ( rangL / 2^n ) * lux_data;
	which rangK is COMMAND-II [1:0] for 0x0=1000,0x1=4000,0x2=16000,0x3=64000
	n is COMMAND-II [3:2] for 0x0=16,0x1=12,0x2=8,0x3=4
	*/

	lux = (lux_data * 1000 )/ 65536;
	lux *= 4;
	/*
	0x01c2 == 6.86;
	0x55bf == 334.9
	*/

	/* LUX range check */
	return lux;
}

static void isl29023_light_enable(struct isl29023_data *data)
{
	isl29023_dbgmsg("starting poll timer, delay %lldns\n",
		    ktime_to_ns(data->light_poll_delay));
	/* push -1 to input subsystem to enable real value to go through next */
	input_report_abs(data->light_input_dev, ABS_MISC, -1);
	hrtimer_start(&data->timer, ktime_set(0, LIGHT_SENSOR_START_TIME_DELAY),
					HRTIMER_MODE_REL);
}

static void isl29023_light_disable(struct isl29023_data *data)
{
	isl29023_dbgmsg("cancelling poll timer\n");
	hrtimer_cancel(&data->timer);
	cancel_work_sync(&data->work_light);
	/* mark the adc buff as not initialized
	 * so that it will be filled again on next light sensor start
	 */
	data->buf_initialized = false;
}

/*
 * SysFS support
 */
static ssize_t isl29023_show_power_state(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct isl29023_data *data = i2c_get_clientdata(to_i2c_client(dev));

	return sprintf(buf, "%u\n", data->power_state);
}

static ssize_t isl29023_store_power_state(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct isl29023_data *data = dev_get_drvdata(dev);
	int ret;
	bool new_value;

	if (sysfs_streq(buf, "1"))
	{
		isl29023_debug = 0;
		new_value = true;
	}
	else if (sysfs_streq(buf, "0"))
	{
		isl29023_debug = 0;
		new_value = false;
	}
	else if (sysfs_streq(buf, "3"))
	{
		isl29023_debug = 1;
		new_value = true;
	}
	else {
		pr_err("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);
	ret = isl29023_set_power_state(data->client, new_value);
	/* Save power state for suspend/resume */
	data->enable = new_value;
	mutex_unlock(&data->update_lock);

	if (ret < 0)
		return ret;

	if (new_value)
		isl29023_light_enable(data);
	else
		isl29023_light_disable(data);

	return count;
}

static DEVICE_ATTR(enable, S_IWUSR | S_IRUGO | S_IWGRP,
		   isl29023_show_power_state, isl29023_store_power_state);

static ssize_t poll_delay_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct isl29023_data *data = dev_get_drvdata(dev);
	return sprintf(buf, "%lld\n", ktime_to_ns(data->light_poll_delay));
}


static ssize_t poll_delay_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct isl29023_data *data = dev_get_drvdata(dev);
	int64_t new_delay;
	int err;

	err = strict_strtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	isl29023_dbgmsg("new delay = %lldns, old delay = %lldns\n",
		    new_delay, ktime_to_ns(data->light_poll_delay));
	mutex_lock(&data->update_lock);
	if (new_delay != ktime_to_ns(data->light_poll_delay)) {
		data->light_poll_delay = ns_to_ktime(new_delay);
		if (data->power_state) {
			isl29023_light_disable(data);
			isl29023_light_enable(data);
		}
	}
	mutex_unlock(&data->update_lock);

	return size;
}

static DEVICE_ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
		   poll_delay_show, poll_delay_store);

static int __isl29023_show_lux(struct i2c_client *client)
{
	u8 lux_lsb, lux_msb;
	int ret;

	ret = i2c_smbus_read_byte_data(client, ISL29023_REG_LSB_SENSOR);

	if (ret < 0)
		return ret;
	lux_lsb = ret;

	ret = i2c_smbus_read_byte_data(client, ISL29023_REG_MSB_SENSOR);

	if (ret < 0)
		return ret;
	lux_msb = ret;

	/* Do the job */
	ret = isl29023_calculate_lux(lux_lsb, lux_msb);
	return ret;
}

static struct attribute *isl29023_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_poll_delay.attr,
	NULL
};

static const struct attribute_group isl29023_attr_group = {
	.attrs = isl29023_attributes,
};

static int lightsensor_get_luxvalue(struct isl29023_data *data)
{
	int i = 0;
	int j = 0;
	unsigned int lux_total = 0;
	int lux_avr_value;
	unsigned int index = 0;
	unsigned int lux_max = 0;
	unsigned int lux_min = 0;
	int value = 0;

	/* get lux value */
	mutex_lock(&data->update_lock);
	value = __isl29023_show_lux(data->client);
	mutex_unlock(&data->update_lock);

	if (value < 0) {
		pr_err("lightsensor returned error %d\n", value);
		return value;
	}
	isl29023_dbgmsg("light value %d\n", value);

	index = (data->index_count++) % BUFFER_NUM;

	/* buffer initialize (light sensor off ---> light sensor on) */
	if (!data->buf_initialized) {
		data->buf_initialized = true;
		for (j = 0; j < BUFFER_NUM; j++)
			data->lux_value_buf[j] = value;
	} else
		data->lux_value_buf[index] = value;

	lux_max = data->lux_value_buf[0];
	lux_min = data->lux_value_buf[0];

	for (i = 0; i < BUFFER_NUM; i++) {
		lux_total += data->lux_value_buf[i];

		if (lux_max < data->lux_value_buf[i])
			lux_max = data->lux_value_buf[i];

		if (lux_min > data->lux_value_buf[i])
			lux_min = data->lux_value_buf[i];
	}
	lux_avr_value = (lux_total-(lux_max+lux_min))/(BUFFER_NUM-2);

	if (data->index_count == BUFFER_NUM)
		data->index_count = 0;

	isl29023_dbgmsg("average light value %d\n", lux_avr_value);
	return lux_avr_value;
}

static void isl29023_work_func_light(struct work_struct *work)
{
	struct isl29023_data *data = container_of(work, struct isl29023_data,
					      work_light);

	int adc = lightsensor_get_luxvalue(data);
	if (adc >= 0) {
		input_report_abs(data->light_input_dev, ABS_MISC, adc);
		input_sync(data->light_input_dev);
	}
}

/* This function is for light sensor.  It operates every a few seconds.
 * It asks for work to be done on a thread because i2c needs a thread
 * context (slow and blocking) and then reschedules the timer to run again.
 */
static enum hrtimer_restart isl29023_timer_func(struct hrtimer *timer)
{
	struct isl29023_data *data = container_of(timer, struct isl29023_data, timer);
	queue_work(data->wq, &data->work_light);
	hrtimer_forward_now(&data->timer, data->light_poll_delay);
	return HRTIMER_RESTART;
}



/*
 * Initialization function
 */

static int isl29023_init_client(struct i2c_client *client)
{
	struct isl29023_data *data = i2c_get_clientdata(client);
	int err;

	err = i2c_smbus_write_byte_data(client,
				   ISL29023_COMMAND1, 0xa0);
	if (err < 0)
		return err;

	err = i2c_smbus_read_byte_data(client, ISL29023_COMMAND1);
	if (err != 0xa0)
		return err;

	printk("%s: OK!\n",__FUNCTION__);

	data->power_state = 1;

	return 0;
}

/*
 * I2C init/probing/exit functions
 */

static struct i2c_driver isl29023_driver;
static int __devinit isl29023_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct isl29023_data *data;
	struct input_dev *input_dev;
	int *opmode, err = -ENODEV;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WRITE_BYTE
					    | I2C_FUNC_SMBUS_READ_BYTE_DATA)) {
		err = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct isl29023_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		pr_err("%s: failed to alloc memory for module data\n",
		       __func__);
		return -ENOMEM;
	}
	data->client = client;
	i2c_set_clientdata(client, data);

	/* Check platform data */
	opmode = client->dev.platform_data;
	if (opmode) {
		if (*opmode < 0 || *opmode > 1) {
			dev_err(&client->dev, "invalid operating_mode (%d)\n",
					*opmode);
			err = -EINVAL;
			goto exit_kfree;
		}
		data->operating_mode = *opmode;
	} else
		data->operating_mode = 0;	/* default mode is standard */
	dev_info(&client->dev, "%s operating mode\n",
			data->operating_mode ? "extended" : "standard");

	/* Initialize the ISL29023 chip */
	err = isl29023_init_client(client);
	if (err)
		goto exit_kfree;

	wake_lock_init(&data->prx_wake_lock, WAKE_LOCK_SUSPEND,
		"prx_wake_lock");
	mutex_init(&data->update_lock);

	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &isl29023_attr_group);
	if (err)
		goto exit_kfree;

	/* hrtimer settings.  we poll for light values using a timer. */
	hrtimer_init(&data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->light_poll_delay = ns_to_ktime(200 * NSEC_PER_MSEC);
	data->timer.function = isl29023_timer_func;

	/* the timer just fires off a work queue request.  we need a thread
	 * to read the i2c (can be slow and blocking)
	 */
	data->wq = create_singlethread_workqueue("isl29023_wq");
	if (!data->wq) {
		err = -ENOMEM;
		pr_err("%s: could not create workqueue\n", __func__);
		goto err_create_workqueue;
	}
	/* this is the thread function we run on the work queue */
	INIT_WORK(&data->work_light, isl29023_work_func_light);

	/* allocate lightsensor-level input_device */
	input_dev = input_allocate_device();
	if (!input_dev) {
		pr_err("%s: could not allocate input device\n", __func__);
		err = -ENOMEM;
		goto err_input_allocate_device_light;
	}
	input_set_drvdata(input_dev, data);
	input_dev->name = "lightsensor-level";
	input_set_capability(input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(input_dev, ABS_MISC, 0, 1, 0, 0);

	dev_info(&client->dev, "registering lightsensor-level input device\n");
	err = input_register_device(input_dev);
	if (err < 0) {
		pr_err("%s: could not register input device\n", __func__);
		input_free_device(input_dev);
		goto err_input_register_device_light;
	}
	data->light_input_dev = input_dev;
	err = sysfs_create_group(&input_dev->dev.kobj,
				 &isl29023_attr_group);
	if (err) {
		pr_err("%s: could not create sysfs group\n", __func__);
		goto err_sysfs_create_group_light;
	}

	dev_info(&client->dev, "support ver. %s enabled\n", DRIVER_VERSION);

	isl29023_light_enable(data);

	return 0;
	/* error, unwind it all */
err_sysfs_create_group_light:
	input_unregister_device(data->light_input_dev);
err_input_register_device_light:
err_input_allocate_device_light:
	destroy_workqueue(data->wq);
err_create_workqueue:
	mutex_destroy(&data->update_lock);
	wake_lock_destroy(&data->prx_wake_lock);
exit_kfree:
	printk("%s: failed!!\n",__FUNCTION__);
	kfree(data);
exit:
	return err;
}

static int __devexit isl29023_remove(struct i2c_client *client)
{
	struct isl29023_data *data = i2c_get_clientdata(client);
	sysfs_remove_group(&data->light_input_dev->dev.kobj,
			   &isl29023_attr_group);

	hrtimer_cancel(&data->timer);
	destroy_workqueue(data->wq);
	input_unregister_device(data->light_input_dev);

	/* Power down the device */
	isl29023_set_power_state(client, 0);

	mutex_destroy(&data->update_lock);
	wake_lock_destroy(&data->prx_wake_lock);

	kfree(i2c_get_clientdata(client));

	return 0;
}

#ifdef CONFIG_PM

static int isl29023_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct isl29023_data *data = i2c_get_clientdata(client);

	if (data->enable)
		isl29023_light_disable(data);

	return isl29023_set_power_state(client, 0);
}

static int isl29023_resume(struct i2c_client *client)
{
	int ret;
	struct isl29023_data *data = i2c_get_clientdata(client);

	ret = isl29023_set_power_state(client, 1);
	if (ret) {
		/* re-enable input events if required */
		if(data->enable)
			isl29023_light_enable(data);
	}

	return ret;
}

#else

#define isl29023_suspend		NULL
#define isl29023_resume		NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id isl29023_id[] = {
	{ "isl29023", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, isl29023_id);

static struct i2c_driver isl29023_driver = {
	.driver = {
		.name	= ISL29023_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.suspend = isl29023_suspend,
	.resume	= isl29023_resume,
	.probe	= isl29023_probe,
	.remove	= __devexit_p(isl29023_remove),
	.id_table = isl29023_id,
};

static int __init isl29023_init(void)
{
	return i2c_add_driver(&isl29023_driver);
}

static void __exit isl29023_exit(void)
{
	i2c_del_driver(&isl29023_driver);
}

MODULE_AUTHOR("Edward Lin <edward.lin@technexion.com>");
MODULE_DESCRIPTION("ISL29023 ambient light sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(isl29023_init);
module_exit(isl29023_exit);
