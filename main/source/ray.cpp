#include "ray.hpp"

/************** RAY ********************/
ray_t::ray_t(player_t *player, float camera_x) : direction({0, 0}), route({0, 0}), 
	delta({INFINITY_, INFINITY_}), start_pos({0,0}), wall_texture_shift(0), 
	map_index_x(0), map_index_y(0), step_x(1), step_y(1), wall_hit(0), side_hit(0)
{
	start_pos = player->position();
	direction.x = player->direction().x + player->camera_vec().x * camera_x;
	direction.y = player->direction().y + player->camera_vec().y * camera_x;
		
	map_index_x = (int)player->position().x;
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
