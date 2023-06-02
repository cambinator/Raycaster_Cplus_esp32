// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "menu.hpp"


/******** elements ********/
element_t::element_t(point_t position, point_t size, const char *text) :
	_text(text), _size(size), _position(position), _text_position(position)
{
	_text_position.y += 2;
	_font = DEFAULT_FONT;
	_font_color = cYELLOW;
    _back_color  = cBLUE;
}

void element_t::set_font(FONT font)
{
	_font = font;
}

void element_t::set_font_color(color16_t color)
{
	_font_color = color;
}

void element_t::set_background_color(color16_t color)
{
	_back_color = color;
}

void element_t::move(point_t new_position){
	_position = new_position;
	_text_position = new_position;
	_text_position.y += 2;
}

void element_t::text_alighn(eTextAlighn pos)
{
	switch (pos){
		case LEFT:
			_text_position.x = _position.x;
			break;
		case CENTER:
			_text_position.x += (_size.x - strlen(_text) * current_font.y_size / 4) / 2 - strlen(_text);
			break;
		case RIGHT:
			_text_position.x += _size.x - strlen(_text) * current_font.y_size / 4 - strlen(_text);
			break;		
		default:
			break;
	}
}

void element_t::draw(device_tft tft)
{
	vSet_Font(_font);
	vSet_Font_Color(&_font_color);
	vSet_Font_Transparency(0);
	vSet_Font_Backgr_Color(&_back_color);
	vSend_Filled_Rect(tft, _position.x, _position.y, _size.x, _size.y, &_back_color);
	xText_Print(tft, _text, _text_position.x, _text_position.y);
}


header_t::header_t(point_t size, const char *text) : element_t({0, 0}, size, text)
{
	_font = DEFAULT_FONT;
	_font_color = cYELLOW;
    _back_color  = cBLUE;
	text_alighn(element_t::eTextAlighn::CENTER);
}


message_t::message_t(point_t position, FONT font, color16_t text_color, const char *text):
	element_t(position, {0, 0}, text)
{
	_font = font;
	_font_color = text_color;
	_back_color = cBLACK;
}


menu_element_t::menu_element_t(point_t position, point_t size, const char* text, uint8_t id) :
	element_t(position, size, text), _frame_color(cMAGENTA), _id(id), _selected(false)
{
	_font = COMIC24_FONT;
	_font_color = cGREEN;
	_back_color  = cBLACK;	
}

void menu_element_t::draw(device_tft tft)
{
	element_t::draw(tft);
	if (_selected){
		vSend_Rect(tft, _position.x, _position.y, _size.x - 2, _size.y, &_frame_color);
	}
}


/********* screens *********/
start_screen_t::start_screen_t()
{
	header = new header_t({lcd_width, 16}, "GAME DEMO");
	message = new message_t({32, 32}, COMIC24_FONT, cGREEN, "RAYCASTER");
	image = new image_t("/spiffs/skull_16b.bin");
	image->move(96, 64);
}

start_screen_t::~start_screen_t()
{
	delete header;
	delete message;
	delete image;
}

void start_screen_t::draw(device_tft tft)
{
	vBlack_Screen(tft);	
	header->draw(tft);
	vTaskDelay(200 / portTICK_PERIOD_MS);
	message->draw(tft);
	vTaskDelay(200 / portTICK_PERIOD_MS);	
	image->draw(tft);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}


end_screen_t::end_screen_t(int score, color16_t color, const char* text, point_t size):
	element_t({0,0}, size, text)
{
	_font = COMIC24_FONT;
	_font_color = cWHITE;
    _back_color  = color;
	_text_position = {32, 32};
	_screen_time = 2000;
	uint32_t nbytes = snprintf(NULL, 0, "Score %d", score) + 1;
	message = (char*)heap_caps_malloc(nbytes, MALLOC_CAP_DEFAULT);
	snprintf(message, nbytes, "Score %d", score);
}

void end_screen_t::draw(device_tft tft)
{
	element_t::draw(tft);
	xText_Print(tft, message, 32, 56);
	vTaskDelay(_screen_time / portTICK_PERIOD_MS);
	vBlack_Screen(tft);
}



/******** main menu ********/
menu_t::menu_t()
{
	header = new header_t({lcd_width, 16}, "select a map");
	elements[0] = new menu_element_t({2, 16}, point_t(lcd_width - 2, 25), "MENU", 0);
	elements[1] = new menu_element_t({2, 16 + 25}, point_t(lcd_width - 2, 25), "MAP1", 0);
	elements[2] = new menu_element_t({2, 16 + 25 * 2}, point_t(lcd_width - 2, 25), "MAP2", 0);
	selected_element = 0;
	elements[selected_element]->select();
}

menu_t::~menu_t()
{
	delete header;
	for (int i = 0; i != MENU_ELEMENTS; ++i){
		delete elements[i];
	}
}

void menu_t::draw(device_tft tft)
{
	header->draw(tft);
	for (int i = 0; i != MENU_ELEMENTS; ++i){
		elements[i]->draw(tft);
	}
}

uint8_t menu_t::loop(device_tft tft)
{
	vBlack_Screen(tft);
	draw(tft);
	bool selected = false;
	while(!selected){
		button_2.check();
		button_6.check();
		bool update_screen = false;
		if (button_2.last_click_number() == 1){
			elements[selected_element]->deselect();
			selected_element++;
			if (selected_element >= MENU_ELEMENTS){
				selected_element = 0;
			}
			elements[selected_element]->select();
			update_screen = true;
		}
		if (button_6.last_click_number() == 1){
			selected = true;
		}
		if (update_screen){
			draw(tft);
		}
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
	return selected_element;
}


