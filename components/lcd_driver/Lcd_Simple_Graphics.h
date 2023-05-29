/* version 2.0.05.11 */
#ifndef LCD_SIMPLE_GRAFICS
#define LCD_SIMPLE_GRAFICS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Lcd_Simple_Driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct point {
	uint16_t x;
	uint16_t y;
} point_t;

typedef enum {DEFAULT_FONT, UBUNTU16_FONT, COMIC24_FONT, DEF_SMALL_FONT} FONT;

extern const color16_t cBLACK;
extern const color16_t cNAVY;
extern const color16_t cDARKGREEN;
extern const color16_t cLIGHTGREY;
extern const color16_t cDARKGREY;
extern const color16_t cBLUE;
extern const color16_t cGREEN;
extern const color16_t cRED;
extern const color16_t cWHITE;
extern const color16_t cYELLOW;
extern const color16_t cMAGENTA;

typedef struct font {
	uint8_t 	*font;
	uint16_t	numchars;
    uint16_t	size;
	uint8_t 	x_size;
	uint8_t 	y_size;
	uint8_t	    offset;
	uint8_t 	max_x_size;
	color16_t*     	color;
	color16_t*     	backgr_color;
	uint8_t     transparent;
} font_t;


extern font_t current_font;  /* Current font structure */

color16_t col_tConvert_Uint8_To_Color16(uint8_t byte);

void vSend_Line_Horizontal(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t length, const color16_t* color);

void vSend_Line_Vertical(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t length, const color16_t* color);

void vSend_Line(device_tft tft, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const color16_t* color);

void vSend_Rect(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const color16_t* color);

void vSend_Filled_Rect(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const color16_t* color);

void vSet_Font(FONT font);

void vSet_Font_Transparency(uint8_t trans);

void vSet_Font_Color(const color16_t* col);

void vSet_Font_Backgr_Color(const color16_t* col);

int16_t iChar_Print(device_tft tft, char c, uint16_t x, uint16_t y);  /* return char width or -1 as error */

int16_t iChar_Print_To_Buf(uint8_t* tft, char c, uint16_t x, uint16_t y);

point_t xText_Print(device_tft tft, const char *string, uint16_t x, uint16_t y);

point_t xText_Print_To_Buf(uint8_t* tft, const char *string, uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif

#endif
