/* version 2.0.05.11 */
#include "Lcd_Simple_Grafics.h"

#define SWAP(a, b) { typeof(a) t = (a); (a) = (b); (b) = (t); }

typedef struct {
    uint16_t adjYOffset;
    uint16_t width;
    uint16_t height;
    uint16_t xOffset;
    uint16_t xDelta;
    uint16_t dataPtr;
    uint8_t charCode;
} single_char_t;

/* Color definitions constants */

const color16_t cBLACK = 0x0000;
const color16_t cNAVY = 0x0F00;
const color16_t cDARKGREEN = 0xC001;
const color16_t cLIGHTGREY = 0x1084;
const color16_t cDARKGREY = 0x4108;
const color16_t cBLUE = 0x1F00;
const color16_t cGREEN = 0xE007;
const color16_t cRED = 0x00F8;
const color16_t cWHITE = 0xDFFF;
const color16_t cYELLOW = 0xC0FF;
const color16_t cMAGENTA = 0x1FF8;

/*  Embedded fonts */
extern uint8_t tft_DefaultFont[];
extern uint8_t tft_Ubuntu16[];
extern uint8_t tft_Comic24[];
extern uint8_t tft_def_small[];


font_t current_font = {
	.font = tft_DefaultFont,
	.numchars = 95,
	.size = 0,
	.x_size = 0,
	.y_size = 0x0B,
	.offset = 0,
	.max_x_size = 0,
	.transparent = 0,
};

static single_char_t fontChar;

/* converting from 8bit to 16 bit color (rrrgggbb). Deprecated */
color16_t IRAM_ATTR col_tConvert_Uint8_To_Color16(uint8_t byte) 
{
	uint16_t red = (byte & 0xE0);
	uint16_t green = (byte & 0x1C) >> 2;
	uint16_t blue = (byte & 0x03) << 11;
	color16_t color = red + green + blue;
	return color;
}


int iRead_File_To_Buf(uint8_t *buffer, const char *fname, uint16_t img_width, uint16_t img_height, uint8_t pix_size)
{
	int bytes_read = 0;
	if (buffer == NULL) {
		puts("iRead_File_To_Buf: buffer not available");
		return 1;
	}
	FILE* file = NULL;
	if (fname){
		file = fopen(fname, "r");
		if (!file){
			printf("iRead_File_To_Buf: cannot open file: %s\n", fname);
			return 1;
		}
		uint32_t file_size = img_width * img_height * pix_size;
		bytes_read = fread(buffer, 1, file_size, file);
		if (bytes_read != file_size){
			printf("iRead_File_To_Buf: wrong file or image side size - %d not %lu\n", bytes_read, file_size);
			fclose(file);
			return 1;
		}
	} else {
		puts("iRead_File_To_Buf: empty filename");
		return 1;
	}
	fclose(file);
	return 0;
}

void vSend_Line_Horizontal(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t length, const color16_t* color)
{
	if (xpos > lcd_width || ypos > lcd_height || length == 0){
		return;
	}
	uint8_t* buffer = NULL;
	buffer = (uint8_t*)heap_caps_malloc(length * PIXEL_SIZE, MALLOC_CAP_DMA);
	if (!buffer){
		puts("vSend_Line_Horizontal: cannot allocate memory");
		return;
	}
	for (int x = 0; x < length; x ++){
		memcpy(buffer + (x * PIXEL_SIZE), color, PIXEL_SIZE);
	}
	vSend_Frame(tft, xpos, ypos, length, 1, buffer);
	free(buffer);
}

void vSend_Line_Vertical(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t length, const color16_t* color)
{
	if (xpos > lcd_width || ypos > lcd_height || length == 0){
		return;
	}
	uint8_t* buffer = NULL;
	buffer = (uint8_t*)heap_caps_malloc(length * PIXEL_SIZE, MALLOC_CAP_DMA);
	if (!buffer){
		puts("vSend_Line_Vertical: cannot allocate memory");
		return;
	}
	for (int x = 0; x < length; x ++){
		memcpy(buffer + (x * PIXEL_SIZE), color, PIXEL_SIZE);
	}
	vSend_Frame(tft, xpos, ypos, 1, length, buffer);
	free(buffer);
}

void vSend_Line(device_tft tft, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const color16_t* color)
{
	if (x0 == x1) {
		if (y0 <= y1){
			vSend_Line_Vertical(tft, x0, y0, y1-y0, color);
		} else {
			vSend_Line_Vertical(tft, x0, y1, y0-y1, color);
		}
		return;
	}
	if (y0 == y1) {
		if (x0 <= x1) {
			vSend_Line_Horizontal(tft, x0, y0, x1-x0, color);
		} else {
			vSend_Line_Horizontal(tft, x1, y0, x0-x1, color);
		}
		return;
	}
	int steep = 0;
	if (abs(y1 - y0) > abs(x1 - x0)){
		steep = 1;
	}
	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}
	int dx = x1 - x0;
	int dy = abs(y1 - y0);
	int err = dx >> 1;
	int ystep = -1;
	int  xs = x0;
	int dlen = 0;

	if (y0 < y1) {
		ystep = 1;
	}
	/*  Split into steep and not steep for FastH/V separation */
	if (steep) {
		for (; x0 <= x1; x0++) {
			dlen++;
			err -= dy;
			if (err < 0) {
				err += dx;
				if (dlen == 1) {
					vSend_Pixel(tft, y0, xs, color);
				} else {
					vSend_Line_Vertical(tft, y0, xs, dlen, color);
				}
				dlen = 0; 
				y0 += ystep; 
				xs = x0 + 1;
			}
		}
		if (dlen){
			vSend_Line_Vertical(tft, y0, xs, dlen, color);
		}
	} else {
		for (; x0 <= x1; x0++) {
			dlen++;
			err -= dy;
			if (err < 0) {
				err += dx;
				if (dlen == 1) {
					vSend_Pixel(tft, xs, y0, color);
				} else {
					vSend_Line_Horizontal(tft, xs, y0, dlen, color);
				}
				dlen = 0; y0 += ystep; xs = x0 + 1;
			}
		}
		if (dlen) {
			vSend_Line_Horizontal(tft, xs, y0, dlen, color);
		}
	}
}

void vSend_Rect(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const color16_t* color)
{
	vSend_Line_Horizontal(tft, xpos, ypos, width, color);
	vSend_Line_Horizontal(tft, xpos, ypos + height - 1, width, color);
	vSend_Line_Vertical(tft, xpos, ypos, height, color);
	vSend_Line_Vertical(tft, xpos + width - 1, ypos, height, color);
}

void vSend_Filled_Rect(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const color16_t* color)
{
	if (ypos > lcd_height || xpos > lcd_width){
		return;
	}
	if ((ypos + height) > lcd_height){
		height = lcd_height - ypos;
	}
	if ((xpos + width) > lcd_width){
		width = lcd_width - xpos;
	}
		
	uint8_t* buffer = NULL;
	buffer = heap_caps_malloc(width * height * PIXEL_SIZE, MALLOC_CAP_DMA);  /* making buffer for whole rectangle */
	if (!buffer){
		puts("vSend_Filled_Rect: cannot allocate memory");
		return;
	}
	for (int x = 0; x < width * height; x ++){
		memcpy(buffer + (x * PIXEL_SIZE), color, PIXEL_SIZE);
	}
	vSend_Frame(tft, xpos, ypos, width, height, buffer);
	free(buffer);
}


static void vGet_Max_Width_Height()
{
	uint16_t tempPtr = 4; /*  point at first char data */
	uint8_t curr_char, char_width, char_heigth, char_delta, char_y_size;
	current_font.numchars = 0;
	current_font.max_x_size = 0;

    curr_char = current_font.font[tempPtr++];
    while (curr_char != 0xFF)  {
    	current_font.numchars++;
        char_y_size = current_font.font[tempPtr++];
        char_width = current_font.font[tempPtr++];
        char_heigth = current_font.font[tempPtr++];
        tempPtr++;
        char_delta = current_font.font[tempPtr++];
        char_y_size += char_heigth;
		if (char_width > current_font.max_x_size){
			current_font.max_x_size = char_width;
		}
		if (char_delta > current_font.max_x_size) {
			current_font.max_x_size = char_delta;
			}
		if (char_heigth > current_font.y_size) {
			current_font.y_size = char_heigth;
		}
		if (char_y_size > current_font.y_size) {
			current_font.y_size = char_y_size;
		}
		if (char_width != 0) {
			/*  packed bits */
			tempPtr += (((char_width * char_heigth) - 1) / 8) + 1;
		}
	    curr_char = current_font.font[tempPtr++];
	}
    current_font.size = tempPtr;
}

static uint8_t uGet_Char_Ptr(uint8_t c) {
	uint16_t tempPtr = 4; /*  point at first char data */
	do {
		fontChar.charCode = current_font.font[tempPtr++];
		if (fontChar.charCode == 0xFF) {
			return 0;
		}
		fontChar.adjYOffset = current_font.font[tempPtr++];
		fontChar.width = current_font.font[tempPtr++];
		fontChar.height = current_font.font[tempPtr++];
		fontChar.xOffset = current_font.font[tempPtr++];
		fontChar.xOffset = fontChar.xOffset < 0x80 ? fontChar.xOffset : -(0xFF - fontChar.xOffset);
		fontChar.xDelta = current_font.font[tempPtr++];
		if (c != fontChar.charCode && fontChar.charCode != 0xFF) {
			if (fontChar.width != 0) {
				/*  packed bits */
				tempPtr += (((fontChar.width * fontChar.height) - 1) / 8) + 1;
			}
		}
	} while ((c != fontChar.charCode) && (fontChar.charCode != 0xFF));
	fontChar.dataPtr = tempPtr;
	return 1;
}

void vSet_Font(FONT font)
{
	current_font.font = NULL;
	if (font == UBUNTU16_FONT) {
		current_font.font = tft_Ubuntu16;
	} else if (font == COMIC24_FONT) {
		current_font.font = tft_Comic24;
	} else if (font == DEF_SMALL_FONT) {
		current_font.font = tft_def_small;
	} else {
		current_font.font = tft_DefaultFont;
	}
	current_font.x_size = current_font.font[0];
	current_font.y_size = current_font.font[1];
	if (current_font.x_size > 0) {
		current_font.offset = current_font.font[2];
		current_font.numchars = current_font.font[3];
		current_font.size = current_font.x_size * current_font.y_size * current_font.numchars;
	} else {
		current_font.offset = 4;
		vGet_Max_Width_Height();
	}
}

static uint16_t iPrint_Prop_Char(device_tft tft, uint16_t x, uint16_t y) 
{
	uint8_t ch = 0;
	int i, j, char_width;
	char_width = ((fontChar.width > fontChar.xDelta) ? fontChar.width : fontChar.xDelta);

	if (!current_font.transparent){	
		int len, bufPos;
		/*  buffer Glyph data for faster sending */
		len = char_width * current_font.y_size;
		uint8_t* color_line = (uint8_t*)heap_caps_malloc(len * PIXEL_SIZE, MALLOC_CAP_DMA);
		if (color_line) {
			/*  fill with background color */
			for (int n = 0; n < len; n++) {
				memcpy(color_line + (n * PIXEL_SIZE), current_font.backgr_color, PIXEL_SIZE);
			}
			/*  set character pixels to foreground color */
			uint8_t mask = 0x80;
			for (j = 0; j < fontChar.height; j++) {
				for (i = 0; i < fontChar.width; i++) {
					if (((i + (j*fontChar.width)) % 8) == 0) {
						mask = 0x80;
						ch = current_font.font[fontChar.dataPtr++];
					}
					if ((ch & mask) != 0) {
						/*  visible pixel */
						bufPos = ((j + fontChar.adjYOffset) * char_width) + (fontChar.xOffset + i);  /*  bufY + bufX */
						memcpy(color_line + (bufPos * PIXEL_SIZE), current_font.color, PIXEL_SIZE);
					}
					mask >>= 1;
				}
			}
			/*  send to display in one transaction */
			vSend_Frame(tft, x, y, char_width, current_font.y_size, color_line);
			free(color_line);
			return char_width;
		}
		return -1;
	} else {
		int cx, cy;
		/*  draw Glyph */
		uint8_t mask = 0x80;
		for (j = 0; j < fontChar.height; j++) {
			for (i = 0; i < fontChar.width; i++) {
				if (((i + (j * fontChar.width)) % 8) == 0) {
					mask = 0x80;
					ch = current_font.font[fontChar.dataPtr++];
				}
				if ((ch & mask) !=0) {
					cx = (uint16_t)(x + fontChar.xOffset+i);
					cy = (uint16_t)(y + j + fontChar.adjYOffset);
					vSend_Pixel(tft, cx, cy, current_font.color);
				}
				mask >>= 1;
			}
		}
		return char_width;
	}
}

static uint16_t iPrint_Prop_Char_To_Buf(uint8_t* tft, uint16_t x, uint16_t y) 
{
	uint8_t ch = 0;
	int i, j, char_width;
	char_width = ((fontChar.width > fontChar.xDelta) ? fontChar.width : fontChar.xDelta);
	int cx, cy;
	/*  draw Glyph */
	uint8_t mask = 0x80;
	for (j = 0; j < fontChar.height; j++) {
		for (i = 0; i < fontChar.width; i++) {
			if (((i + (j * fontChar.width)) % 8) == 0) {
				mask = 0x80;
				ch = current_font.font[fontChar.dataPtr++];
			}
			if ((ch & mask) !=0) {
				cx = (uint16_t)(x + fontChar.xOffset+i);
				cy = (uint16_t)(y + j + fontChar.adjYOffset);
				memcpy(tft + (cy * lcd_width + cx) * PIXEL_SIZE, current_font.color, PIXEL_SIZE);
			}
			mask >>= 1;
		}
	}
	return char_width;
}


void vSet_Font_Transparency(uint8_t trans)
{
	current_font.transparent = trans;
}

void vSet_Font_Color(const color16_t* col)
{
	current_font.color = col;
}

void vSet_Font_Backgr_Color(const color16_t* col)
{
	current_font.backgr_color = col;
}

int16_t iChar_Print(device_tft tft, char c, uint16_t x, uint16_t y)
{
	uint16_t next_pos = 0;
	if(!uGet_Char_Ptr(c)){
		return next_pos;
	}
	next_pos = iPrint_Prop_Char(tft, x, y);
	return next_pos;
}

int16_t iChar_Print_To_Buf(uint8_t* tft, char c, uint16_t x, uint16_t y)
{
	uint16_t next_pos = 0;
	if(!uGet_Char_Ptr(c)){
		return next_pos;
	}
	next_pos = iPrint_Prop_Char_To_Buf(tft, x, y);
	return next_pos;
}

point_t xText_Print(device_tft tft, const char *string, uint16_t x, uint16_t y)
{
	point_t res = {0, 0};
	int ptr = 0;
	uint16_t next_x = x;
	uint16_t next_y = y;
	char c = string[ptr++];
	while (c != '\0'){
		if (c == '\n'){
			next_x = x;
			next_y += current_font.y_size;
		}
		else{
			int16_t curr_char_width = iChar_Print(tft, c, next_x, next_y);
			if (curr_char_width >= 0){
				next_x += (uint16_t)curr_char_width;
			} else {
				puts("xText_Print: char print error");
			}
		}
		c = string[ptr++];
	}
	res.x = next_x;
	res.y = next_y;
	return res;
}

point_t xText_Print_To_Buf(uint8_t* tft, const char *string, uint16_t x, uint16_t y)
{
	point_t res = {0, 0};
	int ptr = 0;
	uint16_t next_x = x;
	uint16_t next_y = y;
	char c = string[ptr++];
	while (c != '\0'){
		if (c == '\n'){
			next_x = x;
			next_y += current_font.y_size;
		}
		else{
			int16_t curr_char_width = iChar_Print_To_Buf(tft, c, next_x, next_y);
			if (curr_char_width >= 0){
				next_x += (uint16_t)curr_char_width;
			} else {
				puts("xText_Print: char print error");
			}
		}
		c = string[ptr++];
	}
	res.x = next_x;
	res.y = next_y;
	return res;
}
