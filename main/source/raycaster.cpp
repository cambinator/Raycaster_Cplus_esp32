// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "raycaster.hpp"

/**************** Support functions *****************************/

/* makes a color 2 times darker */
color16_t IRAM_ATTR Color_Shade(color16_t color)					
{
	/* byte order is little endian, so pixel is stored as 'gggbbbbb|rrrrrggg' */

	/* firstly change the byte order*/
	color = (color << 8) | (color >>8);		

	/* applying a mask to prevent a color shift after the division */
	color &= 0xF7DE;											
	color /= 2;

	/* reverse the byte order again */
	color = (color << 8) | (color >>8); 
	
	return color;
}

/* makes a color 1 bit darker, works for grey colors */
color16_t IRAM_ATTR Color_Darker(color16_t color, int step)  
{	
	color16_t new_col = (color << 8) | (color >>8); 

	/* 0x0841 provides minimal brightness step for a 16bit color */
	new_col -= 0x0841 * step;			

	color = (new_col << 8) | (new_col >>8); 

	return color;
}

/* makes a color 1 bit brighter, works for grey colors */
color16_t IRAM_ATTR Color_Brighter(color16_t color, int step)  
{
	if (step){
		color16_t new_col = (color << 8) | (color >>8); 
		new_col += 0x0841 * step;
		color = (new_col << 8) | (new_col >>8); 
	}
	return color;
}

color16_t IRAM_ATTR Color_Mix(color16_t color_a, color16_t color_b)
{
	color_a = (color_a << 8) | (color_a >>8); 
	color_b = (color_b << 8) | (color_b >>8); 
	color_a = (color_a & 0xF79E) / 2;
	color_b = (color_b & 0xF79E) / 2;
	color16_t res = color_a + color_b;
	return ((res << 8) | (res >>8));
}

color16_t IRAM_ATTR Blue_Gradient(int step)
{
	//'gggbbbbb|rrrrrggg'
	return (0x0100 * (step & 0x1F));
}

color16_t IRAM_ATTR Red_Gradient(int step)
{
	//'gggbbbbb|rrrrrggg'
	return (0x0008 * (step & 0x1F));
}


/************************** Raycaster **********************************/
raycaster_t::raycaster_t(device_tft tft, uint16_t tft_width, uint16_t tft_height, player_t* plr) :
	player(plr), screen_handle(tft), screen_width(tft_width), screen_height(tft_height), pitch(0), player_tile(0)
{
	screen_buffer = (color16_t*)heap_caps_calloc(1, tft_width * tft_height * sizeof(color16_t), MALLOC_CAP_DMA);
}

raycaster_t::~raycaster_t()
{
	free(screen_buffer);
}

void raycaster_t::display()
{
	if (screen_buffer != nullptr){
		vSend_Frame(screen_handle, 0, 0, screen_width, screen_height, (uint8_t*)screen_buffer);
	}
}

void raycaster_t::fill_background()
{
	/********** ceiling *********/
	/* the gradient of the background changes when camera changes its vertical position */
	int adj = (int)player->height() / 32;
	color16_t curr_color = Color_Brighter(cLIGHTGREY, adj / 2);
	#ifdef LIGHT_EFFECTS
	if (player_tile == FOG || player_tile == LAMP){
		curr_color = Color_Mix(curr_color, cWHITE);
	}
	#endif
	for (int i = 0; i < ((screen_height + 2 * pitch) * screen_width) / 2; i ++){
		screen_buffer[i] = curr_color;
		if (i % (screen_width * 4) == 0){
			curr_color = Color_Darker(curr_color, 1);
		}
	}
	//floor
	curr_color = cDARKGREY;
	#ifdef LIGHT_EFFECTS
	if (player_tile == FOG || player_tile == LAMP){
		curr_color = Color_Mix(curr_color, cLIGHTGREY);
	} else if (player_tile == DARK){
		curr_color = cBLACK;
	}
	#endif
	for (int i = ((screen_height + 2 * pitch) * screen_width) / 2; i < (screen_height * screen_width); i ++){
		screen_buffer[i] = curr_color;
		if (i % (screen_width * 4) == 0){
			if (adj <= 0)
				curr_color = Color_Brighter(curr_color, 1);
			else
				adj--;
		}
	}
}

void raycaster_t::draw_map(map_t *map, list_t *doors, texture_t *textures[])
{
	for(int x = 0; x < screen_width; x++){
		/************ casting a ray ************************/
		float camera_x = 2.0f * x / (float)(screen_width) - 1.0f;
		ray_t ray(player, camera_x);
		uint8_t curr_tile = ray.cast(map, doors);	
		float dist_to_wall = ray.distance();
		z_buffer[x] = dist_to_wall;
				
		/********** calculating wall height ***************/		
		/* using shifting left to avoid the float division and save the precision, */
		/* when the distance to the wall is less than 1 */
		/* this little trick saved 3-4 ms per frame */
		/* esp32 is not very quick in floating point calculations */
		const float int_shift = 65536.f;				//2 ^ 16
		uint32_t shifted_dist_to_wall = (uint32_t)(dist_to_wall * int_shift);

		/* calculation of the wall height depending on the distance to the wall */
		uint32_t line_hight = ( (screen_height << 16) / shifted_dist_to_wall);
		uint32_t camera_height = pitch + ((uint32_t(player->height())<<16) / shifted_dist_to_wall);
		int wall_ceiling = (screen_height / 2) - line_hight + camera_height;
		if (wall_ceiling < 0){
			wall_ceiling = 0;
		}
		int wall_floor = screen_height - wall_ceiling + 2 * camera_height;
		if (wall_floor > screen_height){
			wall_floor = screen_height;
		}				
		/***************** wall texturing ********************/
		int tex_index = curr_tile - 8;
		if (tex_index < 0) {
			tex_index = 0;
		}
		if (tex_index > 7) {				/* doors have the same texture, #7 */
			tex_index = 7;
		}			
		int tex_width = textures[tex_index]->width();
		int tex_height = textures[tex_index]->height();
		
		/* calculation of the exact point on the cell, that was hit with the ray */
		float wall_x;					
		if (ray.side() == 0){
			wall_x = player->position().y + dist_to_wall * ray.get_direction().y;
		} else {
			wall_x = player->position().x + dist_to_wall * ray.get_direction().x;
		}			
		wall_x -= floorf(wall_x);
		
		/* sampling texture - getting texture x coordinate,  */
		int tex_x = (int)((wall_x - ray.texture_shift()) * tex_width);
		if (ray.side() == 0 && ray.get_direction().x > 0){
			tex_x = tex_width - tex_x - 1;
		}
		if (ray.side() == 1 && ray.get_direction().y < 0){
			tex_x = tex_width - tex_x - 1;
		}			
		uint32_t step = (tex_height << 16) / (line_hight < 1 ? 1 : line_hight);
		/*  Starting texture y coordinate */
		uint32_t tex_pos = (wall_ceiling - camera_height - screen_height / 2 + line_hight / 2) * step;		
		
		/************** drawing to buffer ****************/
		int ptr = (wall_ceiling * screen_width + x % screen_width);

		for (int i = wall_ceiling; i < wall_floor; i++){
			uint32_t tex_y = (tex_pos >> 16) & (tex_height - 1);
			tex_pos += step;	
			color16_t curr_color = textures[tex_index]->get_pixel(tex_x, tex_y);
			/* shading y side of the wall */
			if (ray.side() == 1){
				curr_color = Color_Shade(curr_color);
			}
			#ifdef LIGHT_EFFECTS
			if (player_tile == FOG || player_tile == LAMP){
				curr_color = Color_Mix(curr_color, cWHITE);
			} else if (player_tile == DARK){
				curr_color = Color_Mix(curr_color, cBLACK);
			}
			#endif
			screen_buffer[ptr] = curr_color;
			ptr += screen_width;
		}
	}
}

void raycaster_t::draw_objects(list_t *objects)
{
	/* calculating the determinant of the inverse matrix */
	float inv_determinant = 1.0f / (player->camera_vec().x * player->direction().y - 
									player->camera_vec().y * player->direction().x);	
	objects->bubble_sort(game_object_t::compare_distances);				/* from further to closer */

	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		if (obj->is_drawable()){
			float rel_pos_x = obj->position().x - player->position().x;
			float rel_pos_y = obj->position().y - player->position().y;
			float transform_x = inv_determinant * (player->direction().y * rel_pos_x - player->direction().x * rel_pos_y);
			float transform_y = inv_determinant * (-player->camera_vec().y * rel_pos_x + player->camera_vec().x * rel_pos_y);		
			int v_screen_shift = (int)(((float)obj->sprite()->vert_shift() + player->height()) / transform_y) + pitch;
			int obj_screen_x = (int)( (screen_width / 2) * (1.0f + transform_x / transform_y));
			int obj_height = abs(2 * (int)(screen_height / transform_y)) / obj->sprite()->size_divider();
			int draw_start_y = -obj_height / 2 + screen_height / 2 + v_screen_shift;
			if (draw_start_y < 0) {
				draw_start_y = 0;
			}
			int draw_end_y = obj_height / 2 + screen_height / 2 + v_screen_shift;
			if(draw_end_y >= screen_height) {
				draw_end_y = screen_height - 1;
			}		
			int obj_width = abs(2 * (int) (screen_height / (transform_y))) / obj->sprite()->size_divider();
			int draw_start_x = -obj_width / 2 + obj_screen_x;
			if(draw_start_x < 0) {
				draw_start_x = 0;
			}
			int draw_end_x = obj_width / 2 + obj_screen_x;
			if(draw_end_x >= screen_width) {
				draw_end_x = screen_width - 1;
			}
			/* adding a health bar */
			int health_bar = 0;
			if((!obj->is_friendly()) && (obj->health() != obj->max_health())){
				health_bar = (obj->health() * obj->sprite()->texture()->width()) / obj->max_health();
			}
			/* draw vertical lines */ 
			for(int x = draw_start_x; x < draw_end_x; x++)	{
				int tex_x = (int)(256 * (x - (-obj_width / 2 + obj_screen_x)) * obj->sprite()->texture()->width() / obj_width) / 256;
				int health_x = tex_x;
				if (obj->sprite()->is_inverted()){
					tex_x = obj->sprite()->texture()->width() - tex_x;
				}
				/* if the object is visible to us and is in front of other objects, and not to close to the camera */
				if(transform_y > 0 && x > 0 && x < screen_width && transform_y < z_buffer[x] && obj->distance() > 0.1){ 
					int buf_ptr = draw_start_y * screen_width + x;				
					for(int y = draw_start_y; y < draw_end_y; y++) { 									/* for every pixel of the current x */
						int d = (y - v_screen_shift) * 256 - screen_height * 128 + obj_height * 128;	/* 256 and 128 factors to avoid floats */
						int tex_y = ((d * obj->sprite()->texture()->height()) / obj_height) / 256;
						color16_t obj_color = obj->sprite()->texture()->get_pixel(tex_x, tex_y);		/* getting the current pixel from the texture */
						if (tex_y == 0 && health_x < health_bar){										/* a health bar on the top of the sprite */
							screen_buffer[buf_ptr] = cRED;
						} else if (!iCompare_Colors(&obj_color, &cBLACK)){								/* cBLACK is considered transparent */
							if (obj->sprite()->is_transparent()){
								obj_color = Color_Mix(screen_buffer[buf_ptr], obj_color);
							} 
							#ifdef LIGHT_EFFECTS
							if (player_tile == FOG || player_tile == LAMP){
								obj_color = Color_Mix(obj_color, cWHITE);
							} else if (player_tile == DARK){
								obj_color = Color_Mix(obj_color, cBLACK);
							}
							#endif
							screen_buffer[buf_ptr] = obj_color;
						}
						buf_ptr += screen_width;
					}//end drawing vertical line
				}
			}//end drawing an object
		}//end object
	}
}

void raycaster_t::draw_player()
{
	if (player->weapon() == player_t::eWeapon::GUN){
		player->images[0]->copy_to_buffer(screen_buffer, screen_width, screen_height);
	}
	else{
	 	if (player->action() == player_t::eAction::ATTACK){
	 		player->images[player_t::eSprite::KNIFE2]->copy_to_buffer(screen_buffer, screen_width, screen_height);
	 	} else{
	 		player->images[player_t::eSprite::KNIFE1]->copy_to_buffer(screen_buffer, screen_width, screen_height);
		}
	}		
	/*********** player's health bar ************/
	for (int i = 0; i != player->health() / 8; i++){
		screen_buffer[3 * screen_width + i + 4] = cRED;
		screen_buffer[4 * screen_width + i + 4] = cRED;
		screen_buffer[5 * screen_width + i + 4] = cRED;
		screen_buffer[6 * screen_width + i + 4] = cRED;
		screen_buffer[7 * screen_width + i + 4] = cRED;
	}
	memset(screen_buffer + (8 * screen_width + 4), 255, 32 * sizeof(color16_t)); 
	
	/* score */
	char text[20];
	sprintf(text, "SCR %d  Amm %u", player->score(), player->ammo());
	xText_Print_To_Buf((uint8_t*)screen_buffer, text, screen_width - ((strlen(text) - 4) * (current_font.y_size)), 2);
	
	/* blood */
	if (player->state() == player_t::eState::DAMAGED){											
		player->images[player_t::eSprite::BLOOD]->copy_to_buffer(screen_buffer, screen_width, screen_height); 
	}
}

void raycaster_t::draw(level_t* level)
{
	player_tile = level->map->get_index((uint16_t)player->position().x, (uint16_t)player->position().y);
	fill_background();
	draw_map(level->map, level->doors, level->textures);
	draw_objects(level->objects);
	draw_player();
	display();
}
