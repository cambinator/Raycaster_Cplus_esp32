#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Graphics.h"

#include "my_list.hpp"
#include "types.hpp"
#include "objects.hpp"
#include "map.hpp"
#include "textures.hpp"
#include "doors.hpp"
#include "player.hpp"
#include "level.hpp"
#include "ray.hpp"

//#define LIGHT_EFFECTS				//enable light effects: fog, darkness
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
	int16_t pitch;
	uint8_t player_tile;
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
