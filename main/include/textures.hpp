#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Graphics.h"
#include "types.hpp"

/************* TEXTURES *****************/
#define NUM_TEX 16

typedef void (*texture_t_generator_fn)(color16_t*, uint8_t, uint8_t);

class texture_t{
private:
	color16_t* _buffer;
	uint8_t _width;
	uint8_t _height;
public:
	texture_t() : _buffer(nullptr), _width(0), _height(0) {}
	~texture_t()
	{
		delete _buffer;
	}
	uint8_t width() const 
	{
		return _width;
	}
	uint8_t height() const
	{
		return _height;
	}
	const color16_t* buffer() const
	{
		return _buffer;
	}
	color16_t get_pixel(uint8_t x, uint8_t y) const;
	color16_t get_pixel_normalized(float x, float y) const;
	void create_generated(const uint8_t width, const uint8_t height, texture_t_generator_fn);
	void create_from_file(const char* filename);
	static void textures_load(texture_t* textures[], int num);
	static void Generate_Tex_1(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h);
	static void Generate_Tex_2(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h);
	static void Generate_Tex_3(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h);
};