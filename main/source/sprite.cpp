// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "sprite.hpp"

/******** SPRITE **********/
sprite_t::sprite_t(texture_t *texture, eSize size, bool transparent, bool inverted, int8_t shift) :
	_texture(texture), _transparent(transparent), _inverted(inverted), _vert_shift(shift)
{
	resize(size);
}

void sprite_t::resize(eSize new_size)
{
	switch (new_size){
		case SMALL:
			_size_divider = 4;
			break;
		case NORMAL:
			_size_divider = 2;
			break;
		case BIG:
			_size_divider = 1;
			break;
		default:
			break;
	}
}