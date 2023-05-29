#include "map.hpp"

uint8_t map_t::get_index(uint16_t x, uint16_t y) const
{
	if (x >= _width || y >= _height){
		return 0;
	} else {
		return _map[y * _width + x];
	}
}

bool map_t::is_solid(uint8_t tile)
{
	return (tile >= eMapCellType::SOLID_START && tile <= eMapCellType::SOLID_END);
}

bool map_t::is_door(uint8_t tile)
{
	return (tile == eMapCellType::DOOR_H || tile == eMapCellType::DOOR_V);
}

map_t *map_t::load_map(uint8_t level)
{
    map_t* map = nullptr;
    switch(level){
		case 1:			
			map = new map_t(map1, map1_width, map1_height);
			break;
		case 2:
			map = new map_t(map2, map2_width, map2_height);
			break;
		case 3:
			map = new map_t(map3, map3_width, map3_height);
			break;
		case 4:
			map = new map_t(map4, map4_width, map4_height);
			break;
		case 5:
			map = new map_t(map5, map5_width, map5_height);
			break;
		default:
			map = new map_t(map1, map1_width, map1_height);
			break;
	}
    return map;
}
