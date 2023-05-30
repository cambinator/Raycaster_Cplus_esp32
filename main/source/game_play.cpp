#include "game_play.hpp"

/**************** GAME ********************/
game_t::game_t(device_tft device, uint8_t start_map_index)
{
	tft = device;

	/* player object */
	player = new player_t();

	/* loading textures */
	texture_t::textures_load(textures, NUM_TEX);

	/* load first level */
	level = new level_t;
	level->load(start_map_index, textures);   

	/* load fonts */
    vSet_Font(DEFAULT_FONT);
	vSet_Font_Color(&cWHITE);
	vSet_Font_Transparency(1);    

	/* raycaster instance */
	raycaster = new raycaster_t(tft, lcd_width, lcd_height, player);
}

void game_t::scene_update()
{
	player->update();
	
	bool check_enemy_near = false;				/* used to select a melee weapon */

	/* iterate through the all game objects and update them */
	for (auto iter = level->objects->begin(); iter != level->objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		obj->update_distance(player);
		obj->on_interact(player);
		if (obj->update(level->map, level->objects, player)){
			player->gain_score(1);
		}
		if (obj->distance() < 2* player->radius() && (!obj->is_friendly()) ){
			check_enemy_near = true;
		}
	}
	if (check_enemy_near){
		player->set_weapon(player_t::eWeapon::KNIFE);
	} else {
		player->set_weapon(player_t::eWeapon::GUN);
	}

	/* update all doors */
	for (auto iter = level->doors->begin(); iter != level->doors->end(); ++iter){
		door_t* door = (door_t*)*iter;
		door->update();
	}	
}

uint8_t game_t::loop()
{
	printf("Let the Game begin\n"); 	
	int heap_free = heap_caps_get_free_size(MALLOC_CAP_DMA); /* debug memory  */
	printf("free memory = %d\n", heap_free);

	vBlack_Screen(tft);

	uint8_t exit_code = 0;							/* 1 - level complete, 2 - death */
	uint32_t global_counter = 0;					/* elapsed time from the start */
	TickType_t accumulator = 0;						/* debug time */
	TickType_t xLastWakeTime = xTaskGetTickCount();	/* init timer */
	/* main loop    */
	for (;;){
		TickType_t xPeriod = xTaskGetTickCount(); 	/* debug time */

		/* reset exit code */
		exit_code = 0;

		/* draw screen */
		raycaster->	draw(level);

		/* read buttons */
		button_1.check();
		button_2.check();
		button_3.check();
		button_4.check();
		button_5.check();
		button_6.check();
		
		/* move the player */
		if (button_1.last_click_number() == 1){
			exit_code = player->move_side(level->map, level->objects, player_t::eDirection::RIGHT);
		}
		if (button_3.last_click_number() == 1 && !exit_code){
			exit_code = player->move_side(level->map, level->objects, player_t::eDirection::LEFT);
		}		
		if (button_4.last_click_number() == 1){
			player->turn(player_t::eDirection::LEFT);
		}			
		if (button_6.last_click_number() == 1){
			player->turn(player_t::eDirection::RIGHT);
		}					
		if (button_2.last_click_number() == 1 && !exit_code){
			exit_code = player->move_forward(level->map, level->objects, level->doors);
		}
		if (button_2.last_click_number() == 2){
			player->jump();
		}		
		if (button_5.last_click_number() == 1){
			player->attack(level->objects, textures);	
		}		

		/* swinging gun up and down */
		if (global_counter % 10 == 0){  
			player->gun_swing();
		}
		/* some event */
		if (global_counter % 128 == 0){ 
			/* int heap_free = heap_caps_get_free_size(MALLOC_CAP_DMA); //debug memory  */
			/* printf("free memory = %d\n", heap_free - MAX_X * MAX_Y * sizeof(color_t)); */
			/* uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL ); //debug memory */
        	/*  printf("watermark 4 = %u\n", uxHighWaterMark); //debug memory */
			printf("mean circle time - %lu ms\n", accumulator / 128);  /* debug time */
			accumulator = 0;		
		}	

		/* main objects and player interaction */
		scene_update();
		
		/* cleaning dead objects */
		game_object_t::clean_objects_list(level->objects);
		
		if (player->state() == player_t::eState::DEAD){
			exit_code = 2;
			break;
		}	

		/* the level is done, loading the next level*/	
		if (exit_code == 1){
			uint8_t next_level = level->next_level();
			if (next_level == 0){
				/* game over, you win */
				break;
			} else {
				/* load the next level*/
				level->load(next_level, textures);
				heap_free = heap_caps_get_free_size(MALLOC_CAP_DMA); /* debug memory  */
				printf("free memory = %d\n", heap_free);
			}
		}	
		global_counter++;
		
		accumulator += (xTaskGetTickCount() - xPeriod) * portTICK_PERIOD_MS;  /* debug time */
					
		vTaskDelayUntil(&xLastWakeTime, ((ms)/portTICK_PERIOD_MS));
	}
	uint16_t exit_score = player->score();

	destroy();

	/******* exit ***********/	
	if (exit_code == 2){
		end_screen_t loose_screen(exit_score, cRED, "Game over");
		loose_screen.draw(tft);
	} else {
		end_screen_t win_screen(exit_score, cDARKGREEN, "You win!");
		win_screen.draw(tft);
	}
	return 0;
}

void game_t::destroy()
{
	delete level;
	delete player;		
	delete raycaster;
	for (int i = 0; i != NUM_TEX; i++){
		delete textures[i];
	}
}
