#pragma once

#include <math.h>
#include "my_list.hpp"
#include "types.hpp"
#include "objects.hpp"
#include "map.hpp"
#include "doors.hpp"
#include "player.hpp"

/**************** RAY ********************/
class ray_t{
private:
	vector_float_t direction;
	vector_float_t route;				/* first distance to the closest intersection */
	vector_float_t delta;				/* distance to the next intersection */
	vector_float_t start_pos;
	float wall_texture_shift;
	int16_t map_index_x;
	int16_t map_index_y;
	int8_t step_x;
	int8_t step_y;
	uint8_t wall_hit;
	uint8_t side_hit;					/*  x or y side of a cell is hit */
public:
	ray_t(player_t* player, float camera_x);
	float distance() const;
	uint8_t cast(map_t* map, list_t* doors);
	const vector_float_t& get_direction() const
	{
		return direction;
	}
	uint8_t side() const
	{
		return side_hit;
	}
	uint8_t wall() const
	{
		return wall_hit;
	}
	float texture_shift() const
	{
		return wall_texture_shift;
	}
};

