#include "raycaster.hpp"

/**************** Support functions *****************************/

color16_t IRAM_ATTR Color_Shade(color16_t color)					/* makes color 2 times darker */
{
	color = (color << 8) | (color >>8); 					/* byte order is little endian, so pixel is stored gggbbbbb|rrrrrggg  */
	color &= 0xF7DE;										/* to prevent overflow */			
	color /= 2;
	color = (color << 8) | (color >>8); 
	return color;
}

color16_t IRAM_ATTR Color_Darker(color16_t color, int step)  /* makes 1 bit darker, works for grey colors */
{	
	color16_t new_col = (color << 8) | (color >>8); 
	new_col -= 0x0841 * step;
	color = (new_col << 8) | (new_col >>8); 
	return color;
}

color16_t IRAM_ATTR Color_Brighter(color16_t color, int step)  /* makes 1 bit lighter, works for grey colors */
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



/************** RAY ********************/
ray_t::ray_t(player_t *player, float camera_x)
{
	start_pos = player->position();
	direction.x = player->direction().x + player->camera_vec().x * camera_x;
	direction.y = player->direction().y + player->camera_vec().y * camera_x;
		
	map_index_x= (int)player->position().x;
	map_index_y = (int)player->position().y;
					
	delta.x = (direction.x == 0) ? INFINITY_ : fabs(1.0f / direction.x);  
	delta.y = (direction.y == 0) ? INFINITY_ : fabs(1.0f / direction.y);  
		
	if (direction.x < 0){
		step_x = -1;
		route.x = (player->position().x - (float)map_index_x) * delta.x;
	} else {
		step_x = 1;
		route.x = ((float)map_index_x + 1.0f - player->position().x) * delta.x;
	}
	if (direction.y < 0){
		step_y = -1;
		route.y = (player->position().y - (float)map_index_y) * delta.y;
	} else {
		step_y = 1;
		route.y = ((float)map_index_y + 1.0f - player->position().y) * delta.y;
	}
}

float ray_t::distance() const
{
    float dist_to_wall = INFINITY_;
	switch(wall_hit){
		case 1: {			/* normal thick wall was hit */
			if (side_hit == 0){
				dist_to_wall = route.x - delta.x;
			} else {
				dist_to_wall = route.y - delta.y;
			} 
			break;
		} 
		case 2:				/* vertical thin wall was hit */
			dist_to_wall = route.x - delta.x / 2.0f;
			break;
		case 3:				/* horisontal thin wall was hit */
			dist_to_wall = route.y - delta.y / 2.0f;
			break;
		default:
			break;
	}	
	return dist_to_wall;
}

uint8_t ray_t::cast(map_t *map, list_t* doors)
{
	uint8_t curr_tile = 0;
	while(!wall_hit){
		if (route.x < route.y){
			route.x += delta.x;
			map_index_x += step_x;
			side_hit = 0;
		} else{
			route.y += delta.y;
			map_index_y += step_y;
			side_hit = 1;
		}
		if (map_index_x < 0 || map_index_y < 0 || map_index_y >= map->height() || map_index_x >= map->width()){			
			break;
		}
		curr_tile = map->get_index(map_index_x, map_index_y);
		
		if (curr_tile >= PORTAL && curr_tile <= SOLID_END)  {				/* thick wall hit */
			wall_hit = 1;
		} else if (curr_tile == DOOR_V){							/* vertical door hit */
			if (route.x - delta.x/2.0f > route.y){
				continue;
			} else {
				float walx = start_pos.y + (route.x - delta.x/2.0f) * direction.y;
				walx -= floorf(walx);
				walx *= (float)DOOR_TIMER_MAX;		
				door_t* door = nullptr;
				for (auto iter = doors->begin(); iter != doors->end(); ++iter){
					door = (door_t*)*iter;
					if (door->position().x == map_index_x && door->position().y == map_index_y){
						break;
					}
				}
				if (door == nullptr){
					wall_hit = 4;
				} else if ((uint8_t)walx < door->time()){
					wall_hit = 2;
					wall_texture_shift = (float)door->time() / (float)DOOR_TIMER_MAX;
				}
			}				
		} else if (curr_tile == DOOR_H){						/* horizontal door hit */
			if (route.y - delta.y / 2.0f > route.x){
				continue;
			} else {
				float walx = start_pos.x + (route.y - delta.y / 2.0f) * direction.x;
				walx -= floorf(walx);
				walx *= (float)DOOR_TIMER_MAX;		
				door_t* door = nullptr;
				for (auto iter = doors->begin(); iter != doors->end(); ++iter){
					door = (door_t*)*iter;
					if (door->position().x == map_index_x && door->position().y == map_index_y){
						break;
					}
				}
				if (door == nullptr){
					wall_hit = 4;
				} else if ((uint8_t)walx < door->time()){
					wall_hit = 3;
					wall_texture_shift = (float)door->time() / (float)DOOR_TIMER_MAX;
				}
			}				
		}
	}
    return curr_tile;
}


/************************** Raycaster **********************************/
raycaster_t::raycaster_t(device_tft tft, uint16_t tft_width, uint16_t tft_height, player_t* plr) :
	player(plr), screen_handle(tft), screen_width(tft_width), screen_height(tft_height)
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
	//ceiling
	int adj = (int)player->height() / 32;
	color16_t curr_color = Color_Brighter(cLIGHTGREY, adj / 2);
	if (player_tile == FOG || player_tile == LAMP){
		curr_color = Color_Mix(curr_color, cWHITE);
	}
	for (int i = 0; i < ((screen_height + 2 * pitch) * screen_width) / 2; i ++){
		screen_buffer[i] = curr_color;
		if (i % (screen_width * 4) == 0){
			curr_color = Color_Darker(curr_color, 1);
		}
	}
	//floor
	curr_color = cDARKGREY;
	if (player_tile == FOG || player_tile == LAMP){
		curr_color = Color_Mix(curr_color, cLIGHTGREY);
	} else if (player_tile == DARK){
		curr_color = cBLACK;
	}
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
	for(int x = 0; x != screen_width; x++){
		/************ casting a ray ************************/
		float camera_x = 2.0f * x / (float)(screen_width) - 1.0f;
		ray_t ray(player, camera_x);
		uint8_t curr_tile = ray.cast(map, doors);	
		float dist_to_wall = ray.distance();
		z_buffer[x] = dist_to_wall;
				
		/********** calculating wall height ***************/		
		float line_hight = (screen_height / (float) dist_to_wall);
		float camera_height = (float)pitch + (player->height() / dist_to_wall);
		int ceiling = (float)(screen_height / 2) - line_hight + camera_height;
		if (ceiling < 0){
			ceiling = 0;
		}
		int flor = screen_height - ceiling + 2 * (camera_height);
		if (flor > screen_height){
			flor = screen_height;
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
		
		float wall_x;						/* exact point on the cell, that was hit with the ray */
		if (ray.side() == 0){
			wall_x = player->position().y + dist_to_wall * ray.get_direction().y;
		} else {
			wall_x = player->position().x + dist_to_wall * ray.get_direction().x;
		}			
		wall_x -= floor(wall_x);
		
		/* sampling texture - getting texture x coordinate,  */
		int tex_x = (int)((wall_x - ray.texture_shift()) * (float)tex_width);
		if (ray.side() == 0 && ray.get_direction().x > 0){
			tex_x = tex_width - tex_x - 1;
		}
		if (ray.side() == 1 && ray.get_direction().y < 0){
			tex_x = tex_width - tex_x - 1;
		}			
		float step = 1.0f * tex_height / (float)line_hight;
		/*  Starting texture y coordinate */
		float tex_pos = (float)(ceiling - camera_height - screen_height / 2 + line_hight / 2) * step;		
		
		/************** drawing to buffer ****************/
		int ptr = (ceiling * screen_width + x % screen_width);

		for (int i = ceiling; i < flor; i++){
			int tex_y = (int)tex_pos & (tex_height - 1);
			tex_pos += step;	
			color16_t curr_color = textures[tex_index]->get_pixel(tex_x, tex_y);
			/* shading y side of the wall */
			if (ray.side() == 1){
				curr_color = Color_Shade(curr_color);
			}
			if (player_tile == FOG || player_tile == LAMP){
				curr_color = Color_Mix(curr_color, cWHITE);
			} else if (player_tile == DARK){
				curr_color = Color_Mix(curr_color, cBLACK);
			}
			screen_buffer[ptr] = curr_color;
			ptr += screen_width;
		}				
	}
}

void raycaster_t::draw_objects(list_t *objects)
{
	/* calculatin determinant of inverse matrix, can be moved out of function, called when player moves; */
	float inv_determinant = 1.0f / (player->camera_vec().x * player->direction().y - 
									player->camera_vec().y * player->direction().x);	
	objects->bubble_sort(game_object_t::compare_distances);				/* from further to closer */

	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		if (obj->sprite()->texture() != nullptr){
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
			/* adding health bar */
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
				/* the conditions in the if are: */
				/* 1) it's in front of camera plane so you don't see things behind you */
				/* 2) it's on the screen (left) */
				/* 3) it's on the screen (right) */
				/* 4) ZBuffer, with perpendicular distance */
				if(transform_y > 0 && x > 0 && x < screen_width && transform_y < z_buffer[x] && obj->distance() > 0.1 /* 0.4*/){ 
					int buf_ptr = draw_start_y * screen_width + x;				
					for(int y = draw_start_y; y < draw_end_y; y++) { 								/* for every pixel of the current x */
						int d = (y - v_screen_shift) * 256 - screen_height * 128 + obj_height * 128;	/* 256 and 128 factors to avoid floats */
						int tex_y = ((d * obj->sprite()->texture()->height()) / obj_height) / 256;
						color16_t obj_color = obj->sprite()->texture()->get_pixel(tex_x, tex_y);				/* get current color from the texture */
						if (tex_y == 0 && health_x < health_bar){									/* health bar */
							screen_buffer[buf_ptr] = cRED;
						} else if (!iCompare_Colors(&obj_color, &cBLACK)){							/*  cBLACK is considered transparent */
							if (obj->sprite()->is_transparent()){
								obj_color = Color_Mix(screen_buffer[buf_ptr], obj_color);
							} 
							if (player_tile == FOG || player_tile == LAMP){
								obj_color = Color_Mix(obj_color, cWHITE);
							} else if (player_tile == DARK){
								obj_color = Color_Mix(obj_color, cBLACK);
							}
							screen_buffer[buf_ptr] = obj_color;
						}
						buf_ptr += screen_width;
					}
				}
			}
		}
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
