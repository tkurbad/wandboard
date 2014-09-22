
#ifndef _edm_h
#define _edm_h

#include <linux/types.h>

extern char *edm_expansion;

extern char *edm_baseboard;

/* External GPIOs correspond to pins 255 -- 264, respectively
*/
#define EDM_N_EXTERNAL_GPIO 10
extern int edm_external_gpio[EDM_N_EXTERNAL_GPIO];

/* CPU internal channels for the I2C buses routed to pins
	0: 37, 39
	1: 231, 233
	2: 235, 237
*/
#define EDM_NOF_I2C 3
extern int edm_i2c[EDM_NOF_I2C];

/* DDC: the internal CPU I2C channel routed to pins HDMI pins 73, 75 */
extern int edm_ddc;

/* CPU internal SPI bus numberings:
	0: 222, 224, 226, 228, 230
	1: 219, 221, 223, 225, 227
*/
#define EDM_NOF_SPI 2
extern int edm_spi[EDM_NOF_SPI];


/* Pointer to a audio structure needed by some boards */
extern void *edm_audio_platform_data;

/* Audio interface abstraction
	Index 0 corresponds to primary I2S, pins 187--195 / 2
	Index 1 corrseponds to secondary I2S, pins 186--194 / 2
	(secondary I2S present on EDM1 only)
   This struct is typically filled in by baseboard, except 'enabled'
   which is filled in by module when I2S data is available.
*/
#define EDM_NOF_I2S 2
struct edm_audio_data_t {
	bool enabled;
	void *platform_data;
#define EDM_BUS_NONE 0
#define EDM_BUS_I2C 1
#define EDM_BUS_SPI 2
#define EDM_BUS_PCI 3
#define EDM_BUS_SERIAL 4
#define EDM_BUS_MMC 5
#define EDM_BUS_USB 6
	int bus_type;
	int bus_number;
	int bus_address;
};
extern struct edm_audio_data_t edm_audio_data[EDM_NOF_I2S];

#endif
