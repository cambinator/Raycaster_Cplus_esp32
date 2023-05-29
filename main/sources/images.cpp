#include "images.hpp"

/*************** IMAGE ****************/

image_t::image_t() : _texture(nullptr), _position({0, 0}) {}

image_t::image_t(const char *filename) : _texture(nullptr), _position({0, 0})
{
	create_from_file(filename);
}

void image_t::create_from_file(const char *filename)
{
	if (_texture != nullptr){
		delete _texture;
	}
	_texture = new texture_t;
	_texture->create_from_file(filename);
}
void image_t::move(uint16_t x, uint16_t y)
{
	_position.x = x;
	_position.y = y;
}

void image_t::copy_to_buffer(color16_t* buff, uint8_t b_width, uint8_t b_height)
{
	if (b_width < width() || b_height < height()){
		puts("Sprite_Copy_To_Buffer: image is too big");
		return;
	}
	uint32_t row_width = width();
	uint32_t buf_ind = (_position.y * b_width + _position.x);		
	for (uint8_t j = 0; j != height(); ++j){
		row_width = width();
		for (uint8_t i = 0; i != width(); ++i) {		
			color16_t color = _texture->get_pixel(i, j);
			if (!iCompare_Colors(&color, &cBLACK)){
				buff[buf_ind] = _texture->get_pixel(i, j);
			}
			buf_ind ++;
			if ((_position.x + i) >= b_width){
				row_width = _position.x + i;
				break;
			}
		}
		buf_ind += (b_width - row_width);
		if (_position.y + j >= b_height){
			break;
		}
	}
}

void image_t::draw(device_tft tft)
{
	if (tft == nullptr || _texture == nullptr){
		return;	
	}
	vSend_Frame(tft, _position.x, _position.y, _texture->width(), _texture->height(), (uint8_t*)_texture->buffer()); 
}