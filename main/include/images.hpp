// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <stdint.h>
#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Graphics.h"
#include "textures.hpp"

/**************** IMAGE *********************/
/* used to store a simple image, that will be directly put on the screen */
class image_t{
private:
	texture_t* _texture;
	point_t _position;				/* in the screen coordinate system */
public:
	image_t();
	explicit image_t(const char* filename);
	~image_t()
	{
		delete _texture;
	}
	const point_t& position() const
	{
		return _position;
	}
	uint8_t width() const
	{
		return _texture->width();
	}
	uint8_t height() const
	{
		return _texture->height();
	}
	void create_from_file(const char* filename);
	void move(uint16_t x, uint16_t y);
	void copy_to_buffer(color16_t* buff, uint8_t b_width, uint8_t b_height);
	void draw(device_tft tft);				/* draw direct to the screen */
};
