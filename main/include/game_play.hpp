// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Graphics.h"

#include "simple_button.hpp"
#include "my_list.hpp"
#include "objects.hpp"
#include "raycaster.hpp"
#include "menu.hpp"
#include "level.hpp"
#include "textures.hpp"
#include "player.hpp"

extern button button_1;  /* move left / enter game */
extern button button_2;  /* move forward/backward */
extern button button_3;  /* move right */
extern button button_4;  /* rotate left */
extern button button_5;  /* shoot / choose game */
extern button button_6;  /* rotate right */

/* handles main game loop and actions order */
class game_t{
private:
	player_t* player;
	texture_t* textures[NUM_TEX];
	level_t* level;
	raycaster_t* raycaster;	
	device_tft tft;
	const int ms = 50;				/* frame time */
private:
	void scene_update();
public:
	explicit game_t(device_tft tft, uint8_t index = 1);
	~game_t() {}
	uint8_t loop();
	void destroy();
};