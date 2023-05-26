/* version 2.0.05.11 */
#ifndef LCD_SIMPLE_DRIVER
#define LCD_SIMPLE_DRIVER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ST7789 commands */
#define ST7789_NOP      0x00
#define ST7789_SWRESET  0x01
#define ST7789_RDDID    0x04
#define ST7789_RDDST    0x09

#define ST7789_RDDPM        0x0A    /*  Read display power mode  */
#define ST7789_RDD_MADCTL   0x0B    /*  Read display MADCTL, type of access to the display coordinates */
#define ST7789_RDD_COLMOD   0x0C    /*  Read display pixel format */
#define ST7789_RDDIM        0x0D    /*  Read display image mode */
#define ST7789_RDDSM        0x0E    /*  Read display signal mode */
#define ST7789_RDDSR        0x0F    /*  Read display self-diagnostic result (ST7789V) */

#define ST7789_SLPIN        0x10
#define ST7789_SLPOUT       0x11
#define ST7789_PTLON        0x12
#define ST7789_NORON        0x13

#define ST7789_INVOFF       0x20
#define ST7789_INVON        0x21
#define ST7789_GAMSET       0x26    /*  Gamma set */
#define ST7789_DISPOFF      0x28
#define ST7789_DISPON       0x29
#define ST7789_CASET        0x2A
#define ST7789_RASET        0x2B
#define ST7789_RAMWR        0x2C
#define ST7789_RGBSET       0x2D    /*  Color setting for 4096, 64K and 262K colors */
#define ST7789_RAMRD        0x2E

#define ST7789_PTLAR        0x30
#define ST7789_VSCRDEF      0x33    /*  Vertical scrolling definition (ST7789V) */
#define ST7789_TEOFF        0x34    /*  Tearing effect line off */
#define ST7789_TEON         0x35    /*  Tearing effect line on */
#define ST7789_MADCTL       0x36    /*  Memory data access control */
#define ST7789_IDMOFF       0x38    /*  Idle mode off */
#define ST7789_IDMON        0x39    /*  Idle mode on */
#define ST7789_RAMWRC       0x3C    /*  Memory write continue (ST7789V) */
#define ST7789_RAMRDC       0x3E    /*  Memory read continue (ST7789V) */
#define ST7789_COLMOD       0x3A

#define ST7789_RAMCTRL      0xB0    /*  RAM control */
#define ST7789_RGBCTRL      0xB1    /*  RGB control */
#define ST7789_PORCTRL      0xB2    /*  Porch control */
#define ST7789_FRCTRL1      0xB3    /*  Frame rate control */
#define ST7789_PARCTRL      0xB5    /*  Partial mode control */
#define ST7789_GCTRL        0xB7    /*  Gate control */
#define ST7789_GTADJ        0xB8    /*  Gate on timing adjustment */
#define ST7789_DGMEN        0xBA    /*  Digital gamma enable */
#define ST7789_VCOMS        0xBB    /*  VCOMS setting */
#define ST7789_LCMCTRL      0xC0    /*  LCM control */
#define ST7789_IDSET        0xC1    /*  ID setting */
#define ST7789_VDVVRHEN     0xC2    /*  VDV and VRH command enable */
#define ST7789_VRHS         0xC3    /*  VRH set */
#define ST7789_VDVSET       0xC4    /*  VDV setting */
#define ST7789_VCMOFSET     0xC5    /*  VCOMS offset set */
#define ST7789_FRCTR2       0xC6    /*  FR Control 2 */
#define ST7789_CABCCTRL     0xC7    /*  CABC control */
#define ST7789_REGSEL1      0xC8    /*  Register value section 1 */
#define ST7789_REGSEL2      0xCA    /*  Register value section 2 */
#define ST7789_PWMFRSEL     0xCC    /*  PWM frequency selection */
#define ST7789_PWCTRL1      0xD0    /*  Power control 1 */
#define ST7789_VAPVANEN     0xD2    /*  Enable VAP/VAN signal output */
#define ST7789_CMD2EN       0xDF    /*  Command 2 enable */
#define ST7789_PVGAMCTRL    0xE0    /*  Positive voltage gamma control */
#define ST7789_NVGAMCTRL    0xE1    /*  Negative voltage gamma control */
#define ST7789_DGMLUTR      0xE2    /*  Digital gamma look-up table for red */
#define ST7789_DGMLUTB      0xE3    /*  Digital gamma look-up table for blue */
#define ST7789_GATECTRL     0xE4    /*  Gate control */
#define ST7789_SPI2EN       0xE7    /*  SPI2 enable */
#define ST7789_PWCTRL2      0xE8    /*  Power control 2 */
#define ST7789_EQCTRL       0xE9    /*  Equalize time control */
#define ST7789_PROMCTRL     0xEC    /*  Program control */
#define ST7789_PROMEN       0xFA    /*  Program mode enable */
#define ST7789_NVMSET       0xFC    /*  NVM setting */
#define ST7789_PROMACT      0xFE    /*  Program action */

#define LCD_HOST    HSPI_HOST

#ifdef CONFIG_WROOM

#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22
#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BCKL 5

#elif defined (CONFIG_TTGO)

#define PIN_NUM_MISO 0
#define PIN_NUM_MOSI 19
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5
#define PIN_NUM_DC   16
#define PIN_NUM_RST  23
#define PIN_NUM_BCKL 4

#endif


#define Y_SHIFT 53
#define X_SHIFT 40

#define MAX_X 240
#define MAX_Y 136


#define PIXEL_SIZE 2

extern uint16_t lcd_width;
extern uint16_t lcd_height;
extern uint16_t y_shift;
extern uint16_t x_shift;
extern uint8_t screen_rotation;

typedef void* device_tft;

enum rotation {PORTRAIT, LANDSCAPE, PORTRAIT_FLIP, LANDSCAPE_FLIP};  

typedef uint16_t color16_t;             /* 16-bit color 5-6-5 */

typedef struct {    
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; 
} lcd_init_cmd_t;


/*Places data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.*/
DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
    /* Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0 */
    {ST7789_MADCTL, {(1<<5)|(1<<7)}, 1},  /*      MX=ML=1, mode LANDSCAPE     MY/MX/MV/ML/RGB/0/0/0/                 */
    /* Interface Pixel Format, 16 bits/pixel for RGB/MCU interface */
    {ST7789_COLMOD, {0x55}, 1},
    /* Porch Setting */
    {ST7789_PORCTRL, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
    /* Gate Control, Vgh=13.65V, Vgl=-10.43V */
    {ST7789_GCTRL, {0x45}, 1},
    /* VCOM Setting, VCOM=1.175V */
    {ST7789_VCOMS, {0x2B}, 1},
    /* LCM Control, XOR: BGR, MX, MH */
    {ST7789_LCMCTRL, {0x2C}, 1},
    /* VDV and VRH Command Enable, enable=1 */
    {ST7789_VDVVRHEN, {0x01, 0xff}, 2},
    /* VRH Set, Vap=4.4+... */
    {ST7789_VRHS, {0x11}, 1},
    /* VDV Set, VDV=0 */
    {ST7789_VDVSET, {0x20}, 1},
    /* Frame Rate Control, 60Hz, inversion=0 */
    {ST7789_FRCTR2, {0x0f}, 1},
    /* Add color inversion */
    {ST7789_INVON, {0}, 0},    
    /* Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V */
    {ST7789_PWCTRL1, {0xA4, 0xA1}, 1},
    /* Positive Voltage Gamma Control */
    {ST7789_PVGAMCTRL, {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19}, 14},
    /* Negative Voltage Gamma Control */
    {ST7789_NVGAMCTRL, {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19}, 14},
    /* Sleep Out */
    {ST7789_SLPOUT, {0}, 0x80},
    /* Display On */
    {ST7789_DISPON, {0}, 0x80},
    {0, {0}, 0xff}
};


/************************ Init commands ***************************************/

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits   */
/* until the transfer is complete.										      */
/* Since command transactions are usually small, they are handled in polling  */
/* mode for higher speed. The overhead of interrupt transactions is more than */
/* just waiting for the transaction to complete.                              */
void vLcd_Send_Cmd(spi_device_handle_t spi, const uint8_t cmd);

/* Send data to the LCD. Uses spi_device_polling_transmit, */ 
/* which waits until the transfer is complete.             */
void vLcd_Send_Data(spi_device_handle_t spi, const uint8_t *data, size_t len);

/* This function is called (in irq context!) just before a transmission starts. */
/* It will set the D/C line to the value indicated in the user field.           */
void vLcd_Spi_Pretransfer_Callback(spi_transaction_t *t);

uint32_t uLcd_Get_ID(spi_device_handle_t spi);

/* Initialize the display */
void vLcd_Init(spi_device_handle_t spi);

void vSet_Screen_Rotation(spi_device_handle_t spi, uint8_t rot);


/****************** Unbuffered drawing functions***********************/

/*Sends a rectangle color buffer to screen */
void _Send_Frame(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata);

/* Sends a rectangle color buffer to screen, black color is transparent */
/* Only for 18bit color */
void _Send_Frame_Transparent(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata);

/* Sends isometric sprite to screen */
void _Send_Frame_Isometric(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata);

/* Sends colored pixel in polling mode, do not needs finishing */
void _Send_Pixel(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, const color16_t* col);

/* clear screen with black color */
void _Black_Screen(spi_device_handle_t spi);


/************************** Queued functions ***************************/ 

/* Sends simple colored pixel. Uses queue, so needs calling vSend_Frame_Finish */
void vSend_Pixel_Queued(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, const color16_t* col);

/* Sends a rectangle color buffer to screen using DMA */
void vSend_Frame_Queued(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata);

/* Finishing one send function */
void vSend_Frame_Finish(spi_device_handle_t spi);

/* finishing group sending */
void vSend_Frame_Group_Finish(spi_device_handle_t spi, int trans_num);


/*******************Internal buffer functions ***************************/
/* These functions work with global buffer, are not using SPI */
void _Send_Frame_To_Buffer(uint8_t* cur_scr_buf, size_t x, size_t y, size_t width, size_t height, const uint8_t* linedata);

void _Send_Pixel_To_Buffer(uint8_t* cur_scr_buf, size_t x, size_t y, const color16_t* color);

void _Send_Frame_Transparent_To_Buffer(uint8_t* cur_scr_buf, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata);

void _Send_Frame_Isometric_To_Buffer(uint8_t* cur_scr_buf, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata);

void _Send_Black_Rect_To_Buffer(uint8_t* cur_scr_buf, size_t x, size_t y, size_t width, size_t height);

void _Black_Screen_To_Buffer(uint8_t* cur_scr_buf);


/***************** Other funcs **********************/

static inline int iCompare_Colors(const color16_t* a, const color16_t* b)
{
	return (*a == *b);
}

static inline bool color_null(const color16_t* color)
{
	return (*color == 0);
}

/**************** INTERFACE *******************************************/

void vSend_Frame(device_tft tft, size_t x, size_t y, size_t width, size_t height, const uint8_t* linedata);

void vBlack_Screen(device_tft tft);

void vSend_Pixel(device_tft tft, size_t x, size_t y, const color16_t* color);

void vSend_Frame_Transparent(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata);

void vSend_Frame_Isometric(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata);

#ifdef __cplusplus
}
#endif

#endif
