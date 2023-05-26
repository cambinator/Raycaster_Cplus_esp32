#include "menu.hpp"

/****************** Drawing headers and interface **********************/
static void Disp_Header(device_tft tft, const char *info)
{
	vSet_Font(DEFAULT_FONT);
	vSet_Font_Color(&cYELLOW);
	vSet_Font_Transparency(0);
	vSet_Font_Backgr_Color(&cBLUE);
	vSend_Filled_Rect(tft, 0, 0, lcd_width, 16, &cBLUE);
	xText_Print(tft, info, ((lcd_width>>1) - (strlen(info)* (current_font.y_size/4))), 2);
}

void End_Screen(device_tft tft, int score, const color16_t* color, const char* text) {
	vSend_Filled_Rect(tft, 0, 0, lcd_width, lcd_height, color);
	vSet_Font(COMIC24_FONT);
	vSet_Font_Color(&cWHITE);
	vSet_Font_Transparency(1);
	xText_Print(tft, text, 32, 32);
	char hdr[10];
	sprintf(hdr, "Score %d", score);
	xText_Print((spi_device_handle_t)tft, hdr, 32, 56);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	vBlack_Screen(tft);
}

void Start_Screen(device_tft tft) {
	vBlack_Screen(tft);
	Disp_Header(tft, "GAME DEMO");
	vSet_Font(COMIC24_FONT);
	vSet_Font_Color(&cGREEN);
	vSet_Font_Transparency(0);
	vSet_Font_Backgr_Color(&cBLACK);
	vTaskDelay(200 / portTICK_PERIOD_MS);
	xText_Print(tft, "RAYCASTER", 32, 32);
	vTaskDelay(200 / portTICK_PERIOD_MS);	
	uint8_t* skull = (uint8_t*)heap_caps_malloc(64 * 64 * 2, MALLOC_CAP_DMA);
    iRead_File_To_Buf(skull, "/spiffs/skull_16b.bin", 64, 64, 2);
    vSend_Frame(tft, 96, 64, 64, 64, skull);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    free(skull);
}

int Start_Menu(device_tft tft)
{
	/* starting new game */
	vBlack_Screen(tft);
	Disp_Header(tft, "Double click to select a map");
	vSet_Font(COMIC24_FONT);
	vSet_Font_Color(&cGREEN);
	vSet_Font_Backgr_Color(&cBLACK);	
	xText_Print(tft, "MENU", 2, 16);
	xText_Print(tft, "MAP 1", 2, 16 + current_font.y_size);
	xText_Print(tft, "MAP 2", 2, 16 + current_font.y_size*2);
	vSend_Rect(tft, 0, 16, lcd_width-2, current_font.y_size, &cMAGENTA);
	int selected_game = 0;
	int previous = 1;
	int selected = 0;
	while (!selected) {
		button_2.check();
		button_6.check();
		if (button_2.last_click_number() == 1){
			previous = selected_game;
			selected_game += 1;
		}
		if (selected_game > 2){
			selected_game = 0;
		}
		vSend_Rect(tft, 0, 16 + (previous * current_font.y_size), lcd_width-2, current_font.y_size, &cBLACK);
		vSend_Rect(tft, 0, 16 + (selected_game * current_font.y_size), lcd_width-2, current_font.y_size, &cMAGENTA);
		if (button_6.last_click_number() == 1){
			selected = 1;
		}
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
	return(selected_game);
}
