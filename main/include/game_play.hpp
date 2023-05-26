#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Grafics.h"

#include "simple_button.hpp"
#include "my_list.hpp"
#include "objects.hpp"
#include "raycaster.hpp"
#include "menu.hpp"


#define NUM_LEVELS 5

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

#ifdef __cplusplus
extern "C" {
#endif

extern button button_1;  /* move left / enter game */
extern button button_2;  /* move forward/backward */
extern button button_3;  /* move right */
extern button button_4;  /* rotate left */
extern button button_5;  /* shoot / choose game */
extern button button_6;  /* rotate right */

class raycaster_t;

class level_t{
public:
	map_t* map;
	list_t* objects;
	list_t* doors;
	texture_t** textures;
private:
	uint8_t curr_level;
public:
	level_t() : map(nullptr), objects(nullptr), doors(nullptr), textures(nullptr), curr_level(1) {}
	~level_t() { }
	void load(uint8_t lev, texture_t** txtrs);
	void destroy();
	uint8_t next_level() const;
};


class game_t{
private:
	player_t* player;
	texture_t* textures[NUM_TEX];
	level_t* level;
	raycaster_t* raycaster;	
	device_tft tft;
	const int ms = 50;				/* frame time */
public:
	explicit game_t(device_tft tft, uint8_t index = 1);
	~game_t() {}
	uint8_t loop();
	void destroy();
};

#ifdef __cplusplus
}
#endif