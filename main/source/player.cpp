// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "player.hpp"

/****************** PLAYER *******************/
player_t::player_t() : 	_dir_vector(PLAYER_START_DIR), _cam_vector(CAM_START_DIR), _position(PLAYER_START_POS),
	_radius(0.2f), angle_velocity(0.1f), velocity(0.1f), _health(MAX_PLAYER_HEALTH), _score(0),  _height(0),
	_ammo(20), _timer(0), _state(eState::GOOD), _action(eAction::STAY), _weapon(eWeapon::GUN), _pistol_shift(1)
{
	/* a picture of the gun in the center of the screen */
	images[GUN] = new image_t;
	images[GUN]->create_from_file("/spiffs/gun_new_16b.bin");
	images[GUN]->move(105, lcd_height - images[GUN]->height() - 1);
	/* blood on the screen, appears when the player receives damage */
	images[BLOOD] = new image_t;
	images[BLOOD]->create_from_file("/spiffs/blood_16b.bin");
	images[BLOOD]->move(90, (lcd_height - images[BLOOD]->height()) / 2);
	images[KNIFE1] = new image_t;
	images[KNIFE1]->create_from_file("/spiffs/knife_16b.bin");
	images[KNIFE1]->move(50, lcd_height - images[KNIFE1]->height() - 1);
	images[KNIFE2] = new image_t;
	images[KNIFE2]->create_from_file("/spiffs/knife2_16b.bin");
	images[KNIFE2]->move(80, lcd_height - images[KNIFE2]->height() - 2);
}

player_t::~player_t()
{
	for (uint8_t i = 0; i != eSprite::COUNT; ++i){
		delete images[i];
	}
}

void player_t::set_weapon(eWeapon wpn)
{
	_weapon = wpn;
}

void player_t::add_ammo(uint8_t bullets)
{
	_ammo += bullets;
	if (_ammo > MAX_AMMO){
		_ammo = MAX_AMMO;
	}
}

void player_t::gun_swing()
{
	images[GUN]->move(images[GUN]->position().x, images[GUN]->position().y + _pistol_shift);
	_pistol_shift = -_pistol_shift;
}

void player_t::update()
{
	if (_state == eState::DAMAGED){
		_state = eState::GOOD;
	}
	if (_height > 0){
		_height -= 10;
	}
	_timer++;
	if (_timer > 2 && _action == eAction::ATTACK){
		_action = eAction::STAY;
		_timer = 0;
	}
}

int player_t::change_health(int level)
{
	int res = 0;
	_health += level;
	if (level < 0){
		_state = eState::DAMAGED;
		res = -1;
	}
	if (_health <= 0){
		_state = eState::DEAD;
		res = -2;
	} else if (_health >= MAX_PLAYER_HEALTH){
		_health = MAX_PLAYER_HEALTH;
		_state = eState::GOOD;
		res = 1;
	}
	return res;
}

bool player_t::check_objects_collision(list_t *objects)
{
    for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		if (obj->diameter() > 0.0f && obj->distance() < 2.0f){		
			obj->update_distance(this);
			if (obj->distance() < obj->diameter()){
				return true;
			}
		}
	}
	return false;
}

bool player_t::check_doors_open(list_t *doors)
{
	bool door_open = false;	
	for (auto iter = doors->begin(); iter != doors->end(); ++iter){
		door_t* door = (door_t*)*iter;
		if (door->position().x == (uint16_t)_position.x && door->position().y == (uint16_t)_position.y){
			if (door->state() == door_t::eState::OPEN){
				door_open = true;
			} else {
				door->open();
			}
			return door_open;
		}
	}	
    return false;
}

bool player_t::map_collision(map_t *map)
{
	/* player's collision rectangle */
    int player_x_h = floorf(_position.x + _radius);  
	int player_x_l = floorf(_position.x - _radius);
	int player_y_h = floorf(_position.y + _radius);
	int player_y_l = floorf(_position.y - _radius);
	if (player_x_l < 0 || player_x_h >= map->width() || player_y_l < 0 || player_y_h >= map->height()){
		return true;
	}
	if (map_t::is_solid(map->get_index(player_x_h, player_y_h)) || map_t::is_solid(map->get_index(player_x_l, player_y_l)) || 
		map_t::is_solid(map->get_index(player_x_h, player_y_l)) || map_t::is_solid(map->get_index(player_x_l, player_y_h))){
		return true;
	}
	return false;
}

uint8_t player_t::move_forward(map_t* map, list_t* objects, list_t* doors)
{
	/* function checks independently a movement in x and y directions to produce a nicer, gliding movement around obstacles */
	float dx = _dir_vector.x * velocity;
	float dy = _dir_vector.y * velocity;
	
	/* move in x direction */
	_position.x += dx;       
	int current_tile = map->get_index((uint16_t)_position.x, (uint16_t)_position.y);	
	
	/* check if on the door tile */
	bool opened_door = false;
	if (map_t::is_door(current_tile)){
		opened_door = check_doors_open(doors);
	}

	if (map_collision(map) || (map_t::is_door(current_tile) && !opened_door)){
		_position.x -= dx;
	}	
	/* check solid objects collision (solid objects that have obj->width > 0) */
	if (check_objects_collision(objects)){
		_position.x -= dx;
	}

	/* move in y direction */
	_position.y += dy;  	
	current_tile = map->get_index((uint16_t)_position.x, (uint16_t)_position.y);
	
	/* check if on the door tile */	
	opened_door = false;
	if (map_t::is_door(current_tile)){
		opened_door = check_doors_open(doors);
	}

	if (map_collision(map) || (map_t::is_door(current_tile) && !opened_door)){
		_position.y -= dy;
	}	
    /* check solid objects collision */
	if (check_objects_collision(objects)){
		_position.y -= dy;
	}
	if (current_tile == PORTAL){		
		/* next level */
		_position = PLAYER_START_POS;
		return 1;									
	}
	return 0;
}

uint8_t player_t::move_side(map_t* map, list_t* objects, eDirection dir)
{
	/* simplified collision check */
	float dx = _dir_vector.y * velocity;
	float dy = _dir_vector.x * velocity;
	if (dir == eDirection::LEFT){
		dx = -dx;
		dy = -dy;
	}
	_position.x -= dx;
	_position.y += dy;
	int current_tile = map->get_index((uint16_t)_position.x, (uint16_t)_position.y);
	if (map_collision(map) || map_t::is_door(current_tile)){
		_position.x += dx;
		_position.y -= dy;
	}	
	if (check_objects_collision(objects)){
		_position.x += dx;
		_position.y -= dy;		
	}
	if (current_tile == PORTAL){
		/* next level */
		_position = PLAYER_START_POS;
		return 1;						
	}	
	return 0;
}

void player_t::turn(eDirection dir)
{
	float vel = angle_velocity;
	if (dir == eDirection::RIGHT){
		vel = -vel;
	}
	vector_float_t old_dir_vector = _dir_vector;
	vector_float_t old_cam_vector = _cam_vector;
	_dir_vector.x = _dir_vector.x * cosf(vel) - _dir_vector.y * sinf(vel);
	_dir_vector.y = old_dir_vector.x * sinf(vel) + _dir_vector.y * cosf(vel);
	_cam_vector.x = _cam_vector.x * cosf(vel) - _cam_vector.y * sinf(vel);
	_cam_vector.y = old_cam_vector.x * sinf(vel) + _cam_vector.y * cosf(vel);
}

void player_t::shoot(list_t *obj_list, texture_t* textures[])
{
	if (_ammo){
		_ammo -= 1;
		bullet_t* bullet = new bullet_t(textures[11], {_position.x + _dir_vector.x, 
			_position.y + _dir_vector.y}, _dir_vector, 64 - _height);
		obj_list->push_back((void*)bullet);	
	}
}

void player_t::knife_attack(list_t *obj_list)
{
	knife_t* knife_sphere = new knife_t(nullptr, _position, {0, 0});
	obj_list->push_back((void*)knife_sphere);	
}

void player_t::attack(list_t *obj_list, texture_t *textures[])
{
	_action = eAction::ATTACK;
	_timer = 0;
	if (_weapon == eWeapon::GUN){
		shoot(obj_list, textures);
	} else if (_weapon == eWeapon::KNIFE){
		knife_attack(obj_list);
	}
}