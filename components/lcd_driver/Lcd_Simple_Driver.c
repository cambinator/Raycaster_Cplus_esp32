/* version 2.0.05.11 */
#include "Lcd_Simple_Driver.h"

uint16_t lcd_width = MAX_X;
uint16_t lcd_height = MAX_Y;
uint16_t y_shift = Y_SHIFT;
uint16_t x_shift = X_SHIFT;
uint8_t screen_rotation = PORTRAIT;
 
void IRAM_ATTR vLcd_Send_Cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
	memset(&t, 0, sizeof(t));       /* Zero out the transaction*/
    t.length = 8;                     /* Command is 8 bits*/
    t.tx_buffer = &cmd;               /* The data is the cmd itself*/
    t.user = (void*)0;                /* D/C needs to be set to 0*/
    ret = spi_device_polling_transmit(spi, &t);  
    assert(ret == ESP_OK);            
}

void IRAM_ATTR vLcd_Send_Data(spi_device_handle_t spi, const uint8_t *data, size_t len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0) return;             
    memset(&t, 0, sizeof(t));         /* Zero out the transaction*/
    t.length = len * 8;               /* Len is in bytes, transaction length is in bits.*/
    t.tx_buffer = data;               /* Data*/
    t.user = (void*)1;                /* D/C needs to be set to 1*/
    ret = spi_device_polling_transmit(spi, &t);  
    assert(ret == ESP_OK);            
}

void IRAM_ATTR vLcd_Spi_Pretransfer_Callback(spi_transaction_t *t) 
{ 
	int dc = (int)t->user; 
	gpio_set_level(PIN_NUM_DC, dc);
}

uint32_t IRAM_ATTR uLcd_Get_ID(spi_device_handle_t spi)
{
    /* get_id cmd*/
    vLcd_Send_Cmd(spi, ST7789_RDDID);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8 * 3;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;

    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    return *(uint32_t*)t.rx_data;
}

void IRAM_ATTR vLcd_Init(spi_device_handle_t spi)
{
    const lcd_init_cmd_t* lcd_init_cmds;
	uint32_t cmd = 0;
    
    /*Initialize non-SPI GPIOs*/
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    /* Reset the display*/
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    /* detect LCD type*/
    uint32_t lcd_id = uLcd_Get_ID(spi);

    printf("LCD ID: %08lX\n", lcd_id);

    printf("LCD ST7789V initialization.\n");
    lcd_init_cmds = st_init_cmds;

    /* Send all the commands*/
    while (lcd_init_cmds[cmd].databytes != 0xff) {
        vLcd_Send_Cmd(spi, lcd_init_cmds[cmd].cmd);
        vLcd_Send_Data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F);
        if (lcd_init_cmds[cmd].databytes & 0x80) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        cmd++;
    }
    /* Enable backlight*/
    gpio_set_level(PIN_NUM_BCKL, 1);
    screen_rotation = LANDSCAPE;  
}

void IRAM_ATTR vSend_Address(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{    
    esp_err_t ret;
    int x;
    static spi_transaction_t trans[4];
    for (x = 0; x != 4; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x & 1) == 0) {
            /* Even transfers are commands*/
            trans[x].length = 8;
            trans[x].user = (void*)0;
        } else {
            /* Odd transfers are data*/
            trans[x].length = 8 * 4;
            trans[x].user = (void*)1;
        }
        trans[x].flags = SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0] = ST7789_CASET;           /* Column Address Set*/
    trans[1].tx_data[0] = (x0) >> 8;              /* Start Col High*/
    trans[1].tx_data[1] = (x0) & 0xff;              /* Start Col Low*/
    trans[1].tx_data[2] = (x0 + width - 1) >> 8;       /* End Col High*/
    trans[1].tx_data[3] = (x0 + width - 1) & 0xff;     /* End Col Low*/
    trans[2].tx_data[0] = ST7789_RASET;           /* Page address set*/
    trans[3].tx_data[0] = (y0) >> 8;        /* Start page high*/
    trans[3].tx_data[1] = (y0) & 0xff;      /* start page low*/
    trans[3].tx_data[2] = (y0 + height - 1) >> 8;    /* end page high*/
    trans[3].tx_data[3] = (y0 + height - 1) & 0xff;  /* end page low*/
    for (x = 0; x != 4; x++) {
    	ret = spi_device_polling_transmit(spi, &trans[x]);
        assert(ret == ESP_OK);
    }
}

void IRAM_ATTR vSend_Raw_Data(spi_device_handle_t spi, const uint8_t* linedata, size_t length) /* length in pixels*/
{    
    esp_err_t ret;
    static spi_transaction_t trans_com;
    static spi_transaction_t trans_data;
    memset(&trans_com, 0, sizeof(spi_transaction_t));
    memset(&trans_data, 0, sizeof(spi_transaction_t));
    
    trans_com.user = (void*)0;
	trans_com.tx_data[0] = ST7789_RAMWR;           /* memory write*/
	trans_com.length = 8;
    trans_com.flags = SPI_TRANS_USE_TXDATA;

	trans_data.user = (void*)1;
    trans_data.tx_buffer = linedata;        /* finally send the line data*/
    trans_data.length = length * PIXEL_SIZE * 8;          /* Data length, in bits*/
    trans_data.flags = 0;
    
    ret=spi_device_polling_transmit(spi, &trans_com);
    assert(ret == ESP_OK);
    
    ret=spi_device_polling_transmit(spi, &trans_data);
    assert(ret == ESP_OK);
}

void IRAM_ATTR vSend_Raw_Col(spi_device_handle_t spi, const color16_t* col) /* rgb color*/
{    
	esp_err_t ret;
    static spi_transaction_t trans_com;
    static spi_transaction_t trans_data;
    memset(&trans_com, 0, sizeof(spi_transaction_t));
    memset(&trans_data, 0, sizeof(spi_transaction_t));
    
    trans_com.length = 8;
    trans_com.user = (void*)0;
    trans_com.flags = SPI_TRANS_USE_TXDATA;
    trans_com.tx_data[0] = ST7789_RAMWR;           /* memory write*/

    trans_data.length = PIXEL_SIZE * 8;
	trans_data.user = (void*)1;
	trans_data.flags = SPI_TRANS_USE_TXDATA;
	trans_data.tx_data[0] = (uint8_t)(*col);
    trans_data.tx_data[1] = ((*col) >> 8) & 0xff;
    
    ret = spi_device_polling_transmit(spi, &trans_com);
    assert(ret == ESP_OK);
    ret = spi_device_polling_transmit(spi, &trans_data);
    assert(ret == ESP_OK);    
}

void _Send_Frame(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata)
{
    if ((xpos > lcd_width) || (ypos > lcd_height)){
    	return;
	}
    xpos += x_shift;
    ypos += y_shift;        
    vSend_Address(spi, xpos, ypos, width, height);
    vSend_Raw_Data(spi, linedata, height * width);
}

/* Only for 18bit color */
void _Send_Frame_Transparent(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata)
{
    if ((xpos > lcd_width) || (ypos > lcd_height)){
    	return;
	}
    xpos += x_shift;
    ypos += y_shift;    
    for (int i = 0; i != height; i++){
		color16_t* temp = (color16_t*)(linedata + i * width * sizeof(color16_t));
		color16_t* start_line;	
    	int start_shift = 0;
    	int line_length = 0;
    	while (color_null(temp++) && start_shift < width){
    		start_shift++;
    	}
    	if (start_shift < width){
    		start_line = temp;
    		while(!color_null(temp++) && ((start_shift + line_length) < width)){
    			line_length++;
    		}
    		vSend_Address(spi, xpos + start_shift, ypos + i, line_length, 1);
    		vSend_Raw_Data(spi, (uint8_t*)start_line, line_length);
    	}
    }
}

void _Send_Frame_Isometric(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata)
{
    if ((xpos > lcd_width) || (ypos > lcd_height)){
    	return;
	}
    xpos += x_shift;
    ypos += y_shift;
    int line_length = 0;
    int line_shift = 0;
    
    for (int i = 0; i != height/2; i++){
    	line_length = 4 * i + 4;
    	line_shift = width/2 - (2 * i + 2);
    	vSend_Address(spi, xpos + line_shift, ypos + i, line_length, 1);
    	vSend_Raw_Data(spi, linedata + ((i * width + line_shift) * PIXEL_SIZE), line_length);
    }
    for (int i = height/2; i != height; i++){
    	line_shift = (2 * i - height);
    	line_length = width - line_shift * 2;
    	vSend_Address(spi, xpos + line_shift, ypos + i, line_length, 1);
    	vSend_Raw_Data(spi, linedata + ((i * width + line_shift) * PIXEL_SIZE), line_length);
    }
}

void _Send_Pixel(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, const color16_t* col)
{
    if ((xpos > lcd_width) || (ypos > lcd_height))
    	return;
    xpos += x_shift;
    ypos += y_shift;
    vSend_Address(spi, xpos, ypos, 1, 1);
    vSend_Raw_Col(spi, col);
}

void _Black_Screen(spi_device_handle_t spi)
{
	#define BUFFER_DIVIDER 8    
	/* dividing on 8 to save memory, very big number results in slower function */
    /*	because of many SPI transaction. */
	/* however, sending whole buffer at once uses a lot of heap */
	/* maximal DMA chunk for SPI transaction should not be more than 4095 bytes */
	vSend_Address(spi, x_shift, y_shift, lcd_width, (lcd_height+1));
	uint8_t* mass = NULL;
	int mass_size = (lcd_width * (lcd_height + 2) * PIXEL_SIZE) / BUFFER_DIVIDER;     
    /* Allocate memory for the pixel buffers */
    mass = heap_caps_calloc(mass_size, 1, MALLOC_CAP_DMA);
    assert(mass != NULL);
       
    vLcd_Send_Cmd(spi, ST7789_RAMWR);
    for (int x = 0; x != BUFFER_DIVIDER; x++){
    	vLcd_Send_Data(spi, mass, mass_size);   
	}
    free(mass);
}        

void IRAM_ATTR vSend_Frame_Queued(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata)
{
    esp_err_t ret;
    int x;
    /* Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this */
    /* function is finished because the SPI driver needs access to it even while we're already calculating the next line. */
    static spi_transaction_t trans[6];
    xpos += x_shift;
    ypos += y_shift;
	
    for (x = 0; x < 6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1) == 0) {
            /* Even transfers are commands */
            trans[x].length = 8;
            trans[x].user = (void*)0;
        } else {
            /* Odd transfers are data */
            trans[x].length = 8 * 4;
            trans[x].user = (void*)1;
        }
        trans[x].flags = SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0] = ST7789_CASET;           /* Column Address Set */
    trans[1].tx_data[0] = (xpos) >> 8;              /* Start Col High */
    trans[1].tx_data[1] = (xpos) & 0xff;              /* Start Col Low */
    trans[1].tx_data[2] = (xpos+width-1) >> 8;       /* End Col High */
    trans[1].tx_data[3] = (xpos+width-1) & 0xff;     /* End Col Low */
    trans[2].tx_data[0] = ST7789_RASET;           /* Page address set */
    trans[3].tx_data[0] = (ypos) >> 8;        /* Start page high */
    trans[3].tx_data[1] = (ypos) & 0xff;      /* start page low */
    trans[3].tx_data[2] = (ypos+height-1) >> 8;    /* end page high */
    trans[3].tx_data[3] = (ypos+height-1) & 0xff;  /* end page low */
    trans[4].tx_data[0] = ST7789_RAMWR;           /* memory write */
    trans[5].tx_buffer = linedata;        /* finally send the line data */
    trans[5].length = width * height * PIXEL_SIZE * 8;          /* Data length, in bits */
    trans[5].flags = 0; /* undo SPI_TRANS_USE_TXDATA flag */

    /* Queue all transactions. */
    for (x = 0; x < 6; x++) {
        ret = spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
        assert(ret == ESP_OK);
    }
    /* When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens */
    /* mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to */
    /* finish because we may as well spend the time calculating the next line. When that is done, we can call */
    /* vSend_Frame_Finish, which will wait for the transfers to be done and check their status. */
    /* Need vSend_Frame_Finish() to be called after. */
	/* actually this function is needed only for sending complex, big buffers, because polling transaction is usually much faster */
}

void IRAM_ATTR vSend_Pixel_Queued(spi_device_handle_t spi, uint16_t xpos, uint16_t ypos, const color16_t* col)
{
    if ((xpos > lcd_width) || (ypos > lcd_height)){
    	return;
	}
    esp_err_t ret;
    int x;
    static spi_transaction_t trans[6];	
	xpos += x_shift;
    ypos += y_shift;

    for (x = 0; x < 6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x & 1)==0) {
            /* Even transfers are commands */
            trans[x].length = 8;
            trans[x].user = (void*)0;
        } else {
            /* Odd transfers are data */
            trans[x].length = 8 * 4;
            trans[x].user = (void*)1;
        }
        trans[x].flags = SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0] = ST7789_CASET;           /* Column Address Set */
    trans[1].tx_data[0] = (xpos) >> 8;              /* Start Col High */
    trans[1].tx_data[1] = (xpos) & 0xff;              /* Start Col Low */
    trans[1].tx_data[2] = (xpos) >> 8;       /* End Col High */
    trans[1].tx_data[3] = (xpos) & 0xff;     /* End Col Low */
    trans[2].tx_data[0] = ST7789_RASET;           /* Page address set */
    trans[3].tx_data[0] = (ypos) >> 8;        /* Start page high */
    trans[3].tx_data[1] = (ypos) & 0xff;      /* start page low */
    trans[3].tx_data[2] = (ypos) >> 8;    /* end page high */
    trans[3].tx_data[3] = (ypos) & 0xff;  /* end page low */
    trans[4].tx_data[0] = ST7789_RAMWR;           /* memory write */
	trans[5].tx_data[0] = ((*col) >> 8) & 0xff;
    trans[5].tx_data[1] = (*col) & 0xff;
	trans[5].length = PIXEL_SIZE * 8;          /* Data length, in bits */
    /* Queue all transactions. */
    for (x = 0; x < 6; x++) {
        ret = spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
        assert(ret == ESP_OK);
    }
}

void vSend_Frame_Finish(spi_device_handle_t spi)
{
    spi_transaction_t* rtrans;
    esp_err_t ret;
    /* Wait for all 6 transactions to be done and get back the results. */
    for (int x = 0; x < 6; x++) {
        ret = spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY); 
        assert(ret == ESP_OK);
        /* We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though. */
    }
}

void vSend_Frame_Group_Finish(spi_device_handle_t spi, int trans_num)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    for (int x = 0; x < trans_num * 6; x++) {
        ret = spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY); 
        assert(ret == ESP_OK);
    }
}

void vSet_Screen_Rotation(spi_device_handle_t spi, uint8_t rot) {
	uint8_t rotation = rot & 3; /*  can't be higher than 3 */
	if (screen_rotation == rotation){
		return;
	}
	
	if ((rotation & 1)) {
        /*  in landscape modes must be width > height */
        lcd_width = MAX_X;
        lcd_height = MAX_Y;
        y_shift = Y_SHIFT;
        x_shift = X_SHIFT;
    }
    else {
        /*  in portrait modes must be width < height */
        lcd_width = MAX_Y;
        lcd_height = MAX_X;
        y_shift = X_SHIFT;
        x_shift = Y_SHIFT;
    }

	uint8_t madctl = 0;
    switch (rotation) {
        case PORTRAIT:
			madctl = (1 << 6) | (1 << 7);
			printf("PORTRAIT\n");
			break;
        case LANDSCAPE:
			madctl = (1 << 5) | (1 << 7);
			y_shift--;
			printf("LANDSCAPE\n");
			break;
        case PORTRAIT_FLIP:
			madctl = 0;
			x_shift--;
			printf("PORTRAIT_FLIP\n");
			break;
        case LANDSCAPE_FLIP:
			madctl = (1 << 5) | (1 << 6);
			printf("LANDSCAPE_FLIP\n");
			break;
		default:
			break;
    }
   	vLcd_Send_Cmd(spi, ST7789_MADCTL);
	vLcd_Send_Data(spi, &madctl, 1);
	screen_rotation = rotation;
}

void _Send_Frame_To_Buffer(uint8_t* cur_scr_buf, size_t x, size_t y, size_t width, size_t height, const uint8_t* linedata)
{
	int buf_ptr = 0;
	int start_pos = y * lcd_width * PIXEL_SIZE + x * PIXEL_SIZE;
	for (int s = 0; s != height; s++){
		memcpy(cur_scr_buf + start_pos, linedata + buf_ptr, width * PIXEL_SIZE);
		start_pos += lcd_width * PIXEL_SIZE;
		buf_ptr += width * PIXEL_SIZE;
	}	 
}

void _Black_Screen_To_Buffer(uint8_t* cur_scr_buf)
{
	memset(cur_scr_buf, 0, MAX_X * MAX_Y * PIXEL_SIZE);
}

void _Send_Pixel_To_Buffer(uint8_t* cur_scr_buf, size_t x, size_t y, const color16_t* color)
{
	int position = y * lcd_width * PIXEL_SIZE + x * PIXEL_SIZE;
	memcpy(cur_scr_buf + position, color, PIXEL_SIZE);
}


void _Send_Frame_Transparent_To_Buffer(uint8_t* cur_scr_buf, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata)
{
    if ((xpos > lcd_width) || (ypos > lcd_height)){
    	return;
	}
    xpos += x_shift;
    ypos += y_shift;
    
    for (int i = 0; i != height; i++){
		color16_t* temp = (color16_t*)(linedata + i * width * sizeof(color16_t));
		color16_t* start_line;
    	int start_shift = 0;
    	int line_length = 0;
    	while (color_null(temp++) && start_shift < width){
    		start_shift++;
    	}
    	if (start_shift < width){
    		start_line = temp;
    		while(!color_null(temp++) && ((start_shift + line_length) < width)){
    			line_length++;
    		}    		
    		int curr_pos = ((ypos + i) * lcd_width + xpos + start_shift) * PIXEL_SIZE;
    		memcpy(cur_scr_buf + curr_pos, start_line, line_length * PIXEL_SIZE);
    	}
    }
}

void _Send_Frame_Isometric_To_Buffer(uint8_t* cur_scr_buf, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata)
{
    if ((xpos > lcd_width) || (ypos > lcd_height)){
    	return;
	}
    int line_length = 0;
    int line_shift = 0;
	xpos += x_shift;
    ypos += y_shift;
    
    for (int i = 0; i != height / 2; i++){
    	line_length = 4 * i + 4;
    	line_shift = width / 2 - (2 * i + 2);    	
    	int curr_pos = ((ypos + i) * lcd_width + xpos + line_shift) * PIXEL_SIZE;
    	memcpy(cur_scr_buf + curr_pos, linedata + ((i * width + line_shift) * PIXEL_SIZE), line_length * PIXEL_SIZE);
    }
    for (int i = height / 2; i != height; i++){
    	line_shift = (2 * i - height);
    	line_length = width - line_shift * 2;    	
    	int curr_pos = ((ypos + i) * lcd_width + xpos + line_shift) * PIXEL_SIZE;
    	memcpy(cur_scr_buf + curr_pos, linedata + ((i * width + line_shift) * PIXEL_SIZE), line_length * PIXEL_SIZE);
    }
}

void _Send_Black_Rect_To_Buffer(uint8_t* cur_scr_buf, size_t x, size_t y, size_t width, size_t height)
{
	int start_pos = (y * lcd_width + x) * PIXEL_SIZE;
	for (int s = 0; s != height; s++){
		memset(cur_scr_buf + start_pos, 0, width * PIXEL_SIZE);
		start_pos += lcd_width * PIXEL_SIZE;
	}	 
}

/***** Interface funcs ******/
void vSend_Frame(device_tft tft, size_t x, size_t y, size_t width, size_t height, const uint8_t* linedata)
{
	_Send_Frame((spi_device_handle_t)tft, x, y, width, height, linedata); 
}

void vBlack_Screen(device_tft tft)
{
	_Black_Screen((spi_device_handle_t)tft);
}

/* only 24 bit color */
void vSend_Pixel(device_tft tft, size_t x, size_t y, const color16_t* color)
{
	_Send_Pixel((spi_device_handle_t)tft, x, y, color); 
}

void vSend_Frame_Transparent(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata)
{
	_Send_Frame_Transparent((spi_device_handle_t)tft, xpos, ypos, width, height, linedata); 
}

void vSend_Frame_Isometric(device_tft tft, uint16_t xpos, uint16_t ypos, uint16_t width, uint16_t height, const uint8_t* linedata)
{
	_Send_Frame_Isometric((spi_device_handle_t)tft, xpos, ypos, width, height, linedata); 
}



