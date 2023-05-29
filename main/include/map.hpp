#pragma once

#include "types.hpp"

#define NUM_MAPS 5

extern const uint8_t map1_width;
extern const uint8_t map1_height;
extern const uint8_t map1[];

extern const uint8_t map2_width;
extern const uint8_t map2_height;
extern const uint8_t map2[];

extern const uint8_t map3_width;
extern const uint8_t map3_height;
extern const uint8_t map3[];

extern const uint8_t map4_width;
extern const uint8_t map4_height;
extern const uint8_t map4[];

extern const uint8_t map5_width;
extern const uint8_t map5_height;
extern const uint8_t map5[];

/**************** MAPS *************************/
class map_t{
private:
	const uint8_t* _map;
	const uint16_t _width;
	const uint16_t _height;
public:
	map_t(const uint8_t* map, const uint16_t width, const uint16_t height) : 
		_map(map), _width(width), _height(height) {}
	uint16_t width() const 
	{
		return _width;
	}
	uint16_t height() const
	{
		return _height;
	}
	uint8_t get_index(uint16_t x, uint16_t y) const;
	static bool is_solid(uint8_t tile);
	static bool is_door(uint8_t tile);
    static map_t* load_map(uint8_t level);
};