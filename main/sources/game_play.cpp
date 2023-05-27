#include "game_play.hpp"

/*************** LEVEL ****************/

void level_t::load(uint8_t lev, texture_t** txtrs)
{
	curr_level = lev;
	printf("load level %u\n", curr_level);
	textures = txtrs;
	switch(curr_level){
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
	objects = new list_t;
	doors = new list_t;
	for (uint16_t j = 0; j != map->height(); j++){
		for (uint16_t i = 0; i!= map->width(); i++){
			switch(map->get_index(i, j)){
				case ZOMBIE : {
					dynamic_t* temp = new dynamic_t(textures[8], {(i + 0.55f), (j + 0.45f)}, {0.03f, 0.01f}, 0, ZOMBIE);
					objects->push_back((void*)temp);
					break;
				}
				case SCORP: {
					dynamic_t* temp = new dynamic_t(textures[9], {(i + 0.55f), (j + 0.45f)}, {0.05f, 0.05f}, 0, SCORP);
					objects->push_back((void*)temp);
					break;
				}
				case BOSS: {
					dynamic_t* temp = new dynamic_t(textures[15], {(i + 0.55f), (j + 0.45f)}, {0.05f, 0.05f}, 0, BOSS);
					objects->push_back((void*)temp);
					break;
				}
				case HEALTH: {
					static_t* temp = new static_t(textures[10], {(i + 0.5f), (j + 0.5f)}, HEALTH);
					objects->push_back((void*)temp);
					break;
				}
				case AMMO: {
					static_t* temp = new static_t(textures[12], {(i + 0.5f), (j + 0.5f)}, AMMO);
					objects->push_back((void*)temp);
					break;
				}
				case BARREL: {
					static_t* temp = new static_t(textures[13], {(i + 0.5f), (j + 0.5f)}, BARREL);
					objects->push_back((void*)temp);
					break;
				}
				case LAMP: {
					static_t* temp = new static_t(textures[14], {(i + 0.5f), (j + 0.5f)}, LAMP);
					objects->push_back((void*)temp);
					break;
				}
				case DOOR_H:
				case DOOR_V: {
					door_t* temp = new door_t(i, j);
					doors->push_back((void*)temp);
					break;
				}
				default:
					break;
			}
		}
	}	
}

void level_t::destroy()
{
	for(auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* object = (game_object_t*)*iter;
		delete object;
	}
	delete objects;
	
	for(auto iter = doors->begin(); iter != doors->end(); ++iter){
		door_t* door = (door_t*)*iter;
		delete door;
	}
	delete doors;

	delete map;
}

uint8_t level_t::next_level() const
{
	if (curr_level >= NUM_LEVELS){
		return 0;
	}
    return (curr_level + 1);
}


/**************** GAME ********************/
game_t::game_t(device_tft device, uint8_t start_map_index)
{
	tft = device;
	/* player object */
	player = new player_t();
	/* loading textures */
	texture_t::textures_load(textures, NUM_TEX);
	/* load level */
	level = new level_t;
	level->load(start_map_index, textures);    
	/* load fonts */
    vSet_Font(DEFAULT_FONT);
	vSet_Font_Color(&cWHITE);
	vSet_Font_Transparency(1);    
	/* raycaster instance */
	raycaster = new raycaster_t(tft, lcd_width, lcd_height, player);
}

uint8_t game_t::loop()
{
	printf("Let the Game begin\n"); 	
	int heap_free = heap_caps_get_free_size(MALLOC_CAP_DMA); /* debug memory  */
	printf("free memory = %d\n", heap_free);

	vBlack_Screen(tft);

	uint8_t exit_code = 0;
	uint32_t global_counter = 0;
	TickType_t accumulator = 0;						/* debug time */
	TickType_t xLastWakeTime = xTaskGetTickCount();	/* init timer */
	/* main loop    */
	for (;;){
		TickType_t xPeriod = xTaskGetTickCount(); 	/* debug time */
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
		
		/* and move player */
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

		if (global_counter % 10 == 0){  /* swinging gun up and down */
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
		/* main interaction */
		game_scene_update(player, level->map, level->objects, level->doors);
		
		/* cleaning dead objects */
		game_object_t::clean_objects_list(level->objects);
		
		if (player->state() == player_t::eState::DEAD){
			level->destroy();
			exit_code = 2; /* death */
			break;
		}		
		if (exit_code == 1){
			level->destroy();
			uint8_t next_level = level->next_level();
			if (next_level == 0){
				break;
			} else {
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
