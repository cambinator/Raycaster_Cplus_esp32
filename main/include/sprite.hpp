#pragma once

#include "types.hpp"
#include "textures.hpp"

/********* sprite **********/
class sprite_t{
public:
	enum eSize : uint8_t {SMALL, NORMAL, BIG};
private:
	texture_t* _texture;
	bool _transparent;
	bool _inverted;
	int8_t _vert_shift;
	uint8_t _size_divider;
public:
	explicit sprite_t(texture_t* texture, eSize size = NORMAL, bool transparent = false, bool inverted = false, int8_t shift = 0);
	void invert()
	{
		_inverted = !_inverted;
	}
	void shift(int8_t new_shift)
	{
		_vert_shift = new_shift;
	}
	bool is_transparent() const
	{
		return _transparent;
	}
	bool is_inverted() const
	{
		return _inverted;
	}
	int8_t vert_shift() const
	{
		return _vert_shift;
	}
	uint8_t size_divider() const
	{
		return _size_divider;
	}
	const texture_t* texture () const
	{
		return _texture;
	}
	void resize(eSize new_size);
};
