/*
 *  Driver for TechNexion Prism dual touch panels
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#define PRISM_MAX_FINGERS 2
#define PRISM_SENSITIVITY 10

#define PRISM_DRIVER_NAME "prism"
static const char *driver_name = PRISM_DRIVER_NAME;
static const char *prism_st_name = PRISM_DRIVER_NAME "_st";

struct prism_event {
        unsigned short x, y;
        unsigned char pressure;
        unsigned char tip;
        unsigned char tid;
};

typedef struct prism_ts_data struct_prism_ts_data;

struct prism_ts_data {
	struct  i2c_client		*client;
	struct  workqueue_struct	*workq;
        struct  work_struct		work;
	struct  input_dev		*mt_dev; /* device for multi-touch */
        struct  input_dev		*st_dev; /* device for single touch */
        int	xmax;
        int 	ymax;
        int     pressure_max;
        int	pendown;
        void	(*adjust)(struct_prism_ts_data *, struct prism_event *);
        int	nof_fingers;
        struct prism_event finger[PRISM_MAX_FINGERS];
        unsigned char save_points;
};

/* ------------------------------------------------------------------------- */

static void prism_cap(struct prism_ts_data *prism, struct prism_event *event) {
        if (event->x > prism->xmax) event->x = prism->xmax;
        if (event->y > prism->ymax) event->y = prism->ymax;
        if (event->pressure > prism->pressure_max) 
                event->pressure = prism->pressure_max;
}

/* ------------------------------------------------------------------------- */

static void prism_cap_flipped(struct prism_ts_data *prism, 
        			struct prism_event *event) {
        event->x = prism->xmax - event->x;
        event->y = prism->ymax - event->y;
        prism_cap(prism, event);
}

/* ------------------------------------------------------------------------- */

static int prism_read_complete(struct prism_ts_data *prism) {
        int ret = i2c_smbus_write_byte_data(prism->client, 0x11, 0);
        if (ret) ret = i2c_smbus_write_byte_data(prism->client, 0x11, 0);
        if (ret) ret = i2c_smbus_write_byte_data(prism->client, 0x11, 0);
        return ret;
}

/* ------------------------------------------------------------------------- */

/*
data from ts is:
YY yy
XX xx
zz
touch id = 1 or 2
touched? = 0 or 1
*/
static int prism_read(struct prism_ts_data *prism) {
	int ret, i, pressure;
	unsigned char data[1+6*PRISM_MAX_FINGERS], *ptr;
	unsigned char msgbuf = 0;
	struct i2c_msg msg[2] = { 
                { prism->client->addr, prism->client->flags, 1, &msgbuf },
                { prism->client->addr, prism->client->flags | I2C_M_RD, sizeof(data), data }
	};

        ret = i2c_transfer(prism->client->adapter, msg, ARRAY_SIZE(msg));        
        if (ret < 0) return ret;

        prism->nof_fingers = data[0] & 3;
        ptr = data+1;
        for (i=0; i<PRISM_MAX_FINGERS; i++) {
                prism->finger[i].y = (ptr[0] << 8) + ptr[1];
                prism->finger[i].x = (ptr[2] << 8) + ptr[3];
                pressure = ((int)ptr[4] * (int)ptr[4]) / (PRISM_SENSITIVITY - 5);
                prism->finger[i].pressure = pressure > 255 ? 255 : pressure;
                prism->finger[i].tip = ptr[5] & 0xf;
                prism->finger[i].tid = ptr[5] >> 4;
                ptr += 6;
        }
	return prism_read_complete(prism);
}

/* ------------------------------------------------------------------------- */

static void prism_work_func(struct work_struct *work) {
	struct prism_ts_data *prism;
        struct prism_event ev[PRISM_MAX_FINGERS] = {{ 0 }};
        int i;
        
        prism = container_of(work, struct prism_ts_data, work);
        prism_read(prism);

        for (i=0; i<PRISM_MAX_FINGERS; i++) {
                if (prism->finger[i].tip)
                        ev[prism->finger[i].tid-1] = prism->finger[i];
        }
        
	if (prism->mt_dev != NULL) {
        	for (i=0; i<PRISM_MAX_FINGERS; i++) {                        
                	if (ev[i].tip) {
                        	prism->adjust(prism, &ev[i]);
                                input_report_abs(prism->mt_dev, ABS_MT_TRACKING_ID, i+1);
                                input_report_abs(prism->mt_dev, ABS_MT_POSITION_X, ev[i].x);
                                input_report_abs(prism->mt_dev, ABS_MT_POSITION_Y, ev[i].y);
			}
                        input_mt_sync(prism->mt_dev);
                }

                input_sync(prism->mt_dev);
	} else

        if (prism->st_dev != NULL) {
                if (ev[0].tip) {
                        int ix = ev[0].x;
                        int iy = ev[0].y;
//                        input_dev_calibrate(prism->st_dev, &ix, &iy);
                        input_report_abs(prism->st_dev, ABS_X, ix);
                        input_report_abs(prism->st_dev, ABS_Y, iy);
                        input_report_abs(prism->st_dev, ABS_PRESSURE, ev[0].pressure);
                        input_sync(prism->st_dev);
                }                
        }
	enable_irq(prism->client->irq);        
}

/* ------------------------------------------------------------------------- */

static irqreturn_t prism_isr(int irq, void *dev_id) {
        struct prism_ts_data *prism = dev_id;

	disable_irq_nosync(prism->client->irq);

	queue_work(prism->workq, &prism->work);
        /*
        schedule_delayed_work(prism->work, HZ/20);
        */
        
	return IRQ_HANDLED;
}

/* ------------------------------------------------------------------------- */

static DECLARE_WORK(prism_work, prism_work_func);

static int prism_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
        struct prism_ts_data *prism;
        int ret;
        int size;
        
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
                return -ENODEV;

        if (client->irq <= 0) return -ENOMEM;
        
        prism = kzalloc(sizeof(*prism), GFP_KERNEL);
        if (prism == NULL) return -ENOMEM;
        
        prism->client = client;
        prism->pendown = 0;
        prism->pressure_max = 255;
	i2c_set_clientdata(client, prism);
        
        ret = i2c_smbus_read_byte_data(client, 0x0e);
        if (ret < 0) goto error;
        
        size = (ret >> 6) & 3;
        switch (size) {
        case 0:
                prism->xmax = 2274;
                prism->ymax = 1274;
                prism->adjust = prism_cap_flipped;
                break;
        case 1:
                prism->xmax = 1499;
                prism->ymax = 899;
                prism->adjust = prism_cap;
                break;
        case 2:
                prism->xmax = 899;
                prism->ymax = 499;
                prism->adjust = prism_cap;
                break;
        case 3:
        default:
                ret = -EINVAL;
                goto error;
        }

#ifdef CONFIG_ANDROID 
        /* For Android, use multi-touch */
        prism->mt_dev = input_allocate_device();
        if (prism->mt_dev != NULL) {
                input_set_abs_params(prism->mt_dev, ABS_MT_POSITION_X, 0, prism->xmax, 0, 0);
                input_set_abs_params(prism->mt_dev, ABS_MT_POSITION_Y, 0, prism->ymax, 0, 0);
                input_set_abs_params(prism->mt_dev, ABS_MT_TRACKING_ID, 0, 15, 0, 0);
                prism->mt_dev->name = driver_name;
                prism->mt_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
                ret = input_register_device(prism->mt_dev);
        } else 
                goto error;
#else
        /* Single touch device is a hack for X servers that does not support multi-touch */        
        prism->st_dev = input_allocate_device();
        if (prism->st_dev != NULL) {
	       prism->st_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
               prism->st_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
               input_set_abs_params(prism->st_dev, ABS_X, 0, prism->xmax, 0, 0);
               input_set_abs_params(prism->st_dev, ABS_Y, 0, prism->ymax, 0, 0);
               input_set_abs_params(prism->st_dev, ABS_PRESSURE, 0, 255, 0, 0);                
               prism->st_dev->name = prism_st_name;
               ret = input_register_device(prism->st_dev);
        } else
                goto error;
#endif /* CONFIG_ANDROID */
        if (ret < 0) goto error_inputdev;

        prism->work = prism_work;
        prism->work.entry.next = prism->work.entry.prev = &prism->work.entry;
        
        prism->workq = create_singlethread_workqueue(PRISM_DRIVER_NAME);
	if (prism->workq == NULL) {
		ret = -ENOMEM;
		goto error_workqueue;
	}

	ret = request_irq(client->irq, prism_isr, IRQF_TRIGGER_RISING, client->name, prism);
	if (ret < 0) goto error_irq;

        ret = prism_read_complete(prism);
	if (ret < 0) goto error_irq_clear;
        
        return 0;
                
        
error_irq_clear:
        free_irq(client->irq, prism);
error_irq:
	destroy_workqueue(prism->workq);	

error_workqueue:
        if (prism->mt_dev) input_unregister_device(prism->mt_dev);
        if (prism->st_dev) input_unregister_device(prism->st_dev);

error_inputdev:
        if (prism->mt_dev) input_free_device(prism->mt_dev);
        if (prism->st_dev) input_free_device(prism->st_dev);

error:
        i2c_set_clientdata(client, NULL);
        kfree(prism);
        return -1;
}

/* ------------------------------------------------------------------------- */

static int prism_remove(struct i2c_client *client) {
        struct prism_ts_data *prism;
        
        prism = dev_get_drvdata(&client->dev);
        i2c_set_clientdata(client, NULL);
	
        if (prism->mt_dev) {
                input_unregister_device(prism->mt_dev);
                input_free_device(prism->mt_dev);
        }
        
        if (prism->st_dev) {
                input_unregister_device(prism->st_dev);
                input_free_device(prism->st_dev);
        }
	
	destroy_workqueue(prism->workq);	
        free_irq(client->irq, prism);
        kfree(prism);
        
        return 0;
}

/* ------------------------------------------------------------------------- */

static struct i2c_device_id prism_id[] = {
	{ PRISM_DRIVER_NAME, 0},
	{},
};

/* ------------------------------------------------------------------------- */
#if CONFIG_PM
static int prism_suspend(struct device *dev) {
        return 0;
}

static int prism_resume(struct device *dev) {
        return 0;
}

static SIMPLE_DEV_PM_OPS(prism_pm, prism_suspend, prism_resume);
#endif

static unsigned short normal_i2c[] = { 0x10, I2C_CLIENT_END };

static struct i2c_driver prism_i2c_driver = {
        .driver = {
                .name 	= PRISM_DRIVER_NAME,
		.bus	= &i2c_bus_type,
	#if CONFIG_PM
		.pm	= &prism_pm,
	#endif
        },
        .probe = prism_probe,
        .remove = prism_remove,
        .id_table = prism_id,
	.address_list   = normal_i2c,
};

static int __init prism_init(void) {
        return i2c_add_driver(&prism_i2c_driver);
}

static void __init prism_exit(void) {
        i2c_del_driver(&prism_i2c_driver);
}

module_init(prism_init);
module_exit(prism_exit);

MODULE_DESCRIPTION("Prism DualTouch Driver");
MODULE_LICENSE("GPL");

