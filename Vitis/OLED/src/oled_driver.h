#ifndef OLED_DRIVER_H
#define OLED_DRIVER_H

#include "xparameters.h"
#include "xgpiops.h"  
#include "xspips.h"  
#include "xstatus.h"

#define GPIO_EMIO 54

#define OLED_VDD    (GPIO_EMIO + 0)
#define OLED_VBAT   (GPIO_EMIO + 1)
#define OLED_RES    (GPIO_EMIO + 2)
#define OLED_DC     (GPIO_EMIO + 3)
#define OLED_LD0    (GPIO_EMIO + 4)
#define OLED_LD1    (GPIO_EMIO + 5)
#define OLED_LD2    (GPIO_EMIO + 6)
#define OLED_LD3    (GPIO_EMIO + 7)

#define CHARGE_PUMP_INIT {0xAE, 0x8D, 0x14, 0xD9, 0xF1}
#define CHARGE_PUMP_INIT_LEN 5

#define CONTROL_INIT {\
    0x81, 0x0F, \
    0xD5, 0x80, \
    0xA8, 0x1F, \
    0xD3, 0x00, \
    0x40, \
    0x20, 0x00, \
    0xDB, 0x40, \
    0xA1, \
    0xC8, \
    0xDA, 0x02, \
    0xA4, \
    0xAF }
#define CONTROL_INIT_LEN 19

#define GPIO_DEVICE_ID  0
#define SPI_DEVICE_ID   0

extern XGpioPs Gpio;
extern XSpiPs  Spi;

int GpioInit(XGpioPs *Gpio, u16 DeviceId);
int SpiInit(XSpiPs *SpiInstancePtr, u16 DeviceId);
void OledCommand(u8 cmd);
void OledData(u8 data);
void OledInit(void);
void OledShutdown(void);
void OledClear(void);
void OledFill(void);
void OledSetCursor(u8 x, u8 y);
void OledPutChar(char c);
void OledPrintString(const char *str);
void OledPrintStringAt(u8 x, u8 y, const char *str);

#endif