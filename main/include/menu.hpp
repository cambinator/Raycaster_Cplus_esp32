#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_button.hpp"
#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Grafics.h"
#include "objects.hpp"

#ifdef __cplusplus
extern "C" {
#endif

extern button button_2;  /* move left / enter game */
extern button button_6;  /* shoot / choose game */

/******** elements ********/

class element_t{
public:
    enum eTextAlighn{LEFT, CENTER, RIGHT};
protected:
    FONT _font;
    const char* _text;
	color16_t _font_color;
    color16_t _back_color;
	point_t _size;
    point_t _position;
    point_t _text_position;
protected:
    void set_font(FONT font);
    void set_font_color(color16_t color);
    void set_background_color(color16_t color);
    void move(point_t new_position);
    void text_alighn(eTextAlighn pos);
    element_t(point_t position, point_t size, const char* text);
public:
    void draw(device_tft tft);
    point_t get_size() const
    {
        return _size;
    }
    point_t get_position() const
    {
        return _position;
    }
};


class header_t : public element_t{
public:
    header_t(point_t size, const char* text);
};


class message_t : public element_t{
public:
    message_t(point_t position, FONT font, color16_t text_color, const char* text);
};


class menu_element_t : public element_t{
private:
    color16_t _frame_color;
    uint8_t _id;
    bool _selected;
public:
    menu_element_t(point_t position, point_t size, const char* text, uint8_t id);
    uint8_t get_id() const
    {
        return _id;
    }
    void select()
    {
        _selected = true;
    }
    void deselect()
    {
        _selected = false;
    }
    void draw(device_tft tft);
};


/********* screens *********/

class start_screen_t{
private:
    header_t* header;
    message_t* message;
    image_t* image;
public:
    start_screen_t();
    ~start_screen_t();
    void draw(device_tft tft);
};


class end_screen_t : public element_t{
private:
    uint32_t _screen_time;
    char* message;
public:
    end_screen_t(int score, color16_t color, const char* text, point_t size = {lcd_width, lcd_height});
    ~end_screen_t()
    {
        delete message;
    }
    void draw(device_tft tft);
};


/******** main menu ********/

#define MENU_ELEMENTS 3

class menu_t{
private:
    header_t* header;
    menu_element_t* elements[MENU_ELEMENTS];
    uint8_t selected_element;
private:
    void draw(device_tft tft);
public:
    menu_t();
    ~menu_t();
    uint8_t loop(device_tft tft);
};



#ifdef __cplusplus
}
#endif