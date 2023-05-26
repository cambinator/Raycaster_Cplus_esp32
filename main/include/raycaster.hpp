#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Grafics.h"

#include "my_list.hpp"
#include "types.hpp"
#include "objects.hpp"
#include "game_play.hpp"

#define ARRAY_LENGTH(_Array) (sizeof(_Array) / sizeof(_Array[0]))


#ifdef __cplusplus
extern "C" {
#endif


/**************** RAY ********************/
class ray_t{
private:
	vector_float_t direction = {1.f, 1.f};
	vector_float_t route = {1.f, 1.f};		/* first distance to the closest intersection */
	vector_float_t delta = {INFINITY_, INFINITY_}; /* distance to the next intersection */
	vector_float_t start_pos = {0,0};
	float wall_texture_shift = 0;
	int16_t map_index_x = 0;
	int16_t map_index_y = 0;
	int8_t step_x = 1;
	int8_t step_y = 1;
	uint8_t wall_hit = 0;
	uint8_t side_hit = 0; /*  x or y side of a cell is hit */
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


/*********************** Raycaster *********************************/
class level_t;

class raycaster_t{
private:
	player_t* player;
	device_tft screen_handle;
	color16_t* screen_buffer;
	float z_buffer[MAX_X];				/* too big to dynamic allocation*/
	uint16_t screen_width;
	uint16_t screen_height;
	int16_t pitch = 0;
	uint8_t player_tile = 0;
private:
	void display();
	void fill_background();
	void draw_map(map_t* map, list_t* doors, texture_t* textures[]);
	void draw_objects(list_t* objects);
	void draw_player();
public:
	raycaster_t(device_tft tft, uint16_t tft_width, uint16_t tft_height, player_t* plr);
	~raycaster_t();
	void draw(level_t* level);
};

#ifdef __cplusplus
}
#endif
