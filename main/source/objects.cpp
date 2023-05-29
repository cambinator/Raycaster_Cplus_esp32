#include "objects.hpp"

/* fast inverse square root, 1.5-2 times quicker on esp32 than sqrt */
float IRAM_ATTR fsqrtf(float x) 
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;
    x = x * (1.5f - xhalf * x * x);
    return x;
}


/****************** OBJECTS ********************/
bool game_object_t::compare_distances(void *a, void *b)
{
    game_object_t* first = (game_object_t*)a;
	game_object_t* second = (game_object_t*)b;
	return (first->distance() < second->distance());
}

game_object_t::game_object_t() : _sprite(nullptr), _position({0.0f, 0.0f}), 
	_distance(INFINITY_), _diameter(0.0f), _max_health(2000), _health(2000),
	_damage(0), _friendly(true), _to_remove(false) {}

game_object_t::game_object_t(texture_t *texture, vector_float_t position, int vert_shift) : 
	_position(position), _distance(INFINITY_), _diameter(0.0f), _max_health(2000), _health(2000), 
	_damage(0), _friendly(true), _to_remove(false)
{
	_sprite = new sprite_t(texture, sprite_t::NORMAL, false, false, vert_shift);
}

void game_object_t::update_distance(player_t *player)
{
	_distance = (player->position().x - _position.x) * (player->position().x - _position.x) +
				(player->position().y - _position.y) * (player->position().y - _position.y);
}

void game_object_t::clean_objects_list(list_t *objects)
{
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		if (obj->to_remove()){
			++iter;
			objects->pop_item(obj);
			delete obj;
			if (iter == nullptr){
				break;
			}
		}
	}
}

int8_t dynamic_t::map_collision(map_t *map)
{
    int obj_x_h = floorf(_position.x + _diameter);  
	int obj_x_l = floorf(_position.x - _diameter);
	int obj_y_h = floorf(_position.y + _diameter);
	int obj_y_l = floorf(_position.y - _diameter);
	if (obj_x_l < 0 || obj_x_h >= map->width() || obj_y_l < 0 || obj_y_h >= map->height()){
		return -1;
	}
	if (map_t::is_solid(map->get_index(obj_x_h, obj_y_h)) || map_t::is_solid(map->get_index(obj_x_l, obj_y_l)) || 
		map_t::is_solid(map->get_index(obj_x_h, obj_y_l)) || map_t::is_solid(map->get_index(obj_x_l, obj_y_h))){
		return 1;
	}
	return 0;
}

/************ dynamic objects **************/
dynamic_t::dynamic_t(texture_t *texture, vector_float_t position, vector_float_t velocity, int v_shift):
	game_object_t(texture, position, v_shift), _velocity(velocity), _timer(0) {}

int8_t dynamic_t::map_move(map_t* map){
	if (_to_remove){
		return -1;
	}
	int8_t collision = 0;
	/* move x */
	_position.x += _velocity.x;
	int8_t map_collision_result = map_collision(map);
	if (map_collision_result == -1){
		remove();
		return -1;
	} else if (map_collision_result == 1){
		_velocity.x = -_velocity.x;
		_position.x += _velocity.x;
		collision = 1;
	}
	/* move y */
	_position.y += _velocity.y;
	map_collision_result = map_collision(map);
	if (map_collision_result == -1){
		remove();
		return -1;
	} else if (map_collision_result == 1){
		_velocity.y = -_velocity.y;
		_position.y += _velocity.y;
		collision = 1;
	}
	return collision;
}


bullet_t::bullet_t(texture_t *texture, vector_float_t position, vector_float_t velocity, int v_shift):
	dynamic_t(texture, position, velocity, v_shift)
{
	_damage = 40;
	_diameter = 0.0f;
}

int bullet_t::update(map_t* map, list_t* objects, player_t* player)
{
	int8_t move_result = map_move(map);
	if (move_result == -1){
		return 0;
	} else if (move_result == 1){
		remove();
		return 0;
	}
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* next_object = (game_object_t*)*iter;
		if (next_object != (game_object_t*)this){			
			float x = fabs(_position.x - next_object->position().x);
			float y = fabs(_position.y - next_object->position().y);
			float radius = _diameter + next_object->diameter();
			if (x <= radius * 2 && y <= radius * 2){
				if (next_object->collide(this)){
					return 1;
				}
			}
		}			
	}
	return 0;
}

knife_t::knife_t(texture_t *texture, vector_float_t position, vector_float_t velocity):
	dynamic_t(texture, position, velocity, 64)
{
	_damage = 60;
	_diameter = 0.0f;
}

int knife_t::update(map_t* map, list_t* objects, player_t* player)
{
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* next_object = (game_object_t*)*iter;
		if (next_object != (game_object_t*)this){			
			float x = fabs(_position.x - next_object->position().x);
			float y = fabs(_position.y - next_object->position().y);
			float radius = _diameter + next_object->diameter();
			if (x <= radius * 2 && y <= radius * 2 && !_to_remove){
				if (next_object->collide(this)){
					return 1;
				}
			}
		}			
	}
	if (_timer > 4){
		_timer = 0;
		remove();
	}
	return 0;
}

enemy_t::enemy_t(texture_t *texture, vector_float_t position, vector_float_t velocity):
	dynamic_t(texture, position, velocity, 64)
{
	_friendly = false;
}

void enemy_t::collide_dynamic_objects(list_t *objects)
{
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* next_object = (game_object_t*)*iter;
		if (next_object != (game_object_t*)this){			
			float x = fabs(_position.x - next_object->position().x);
			float y = fabs(_position.y - next_object->position().y);
			float radius = _diameter + next_object->diameter();
			if (x <= radius * 2 && y <= radius * 2){
				if (next_object->diameter() > 0.2f){				/* passing through small objects.. */
					_velocity.x = -_velocity.x;
					_velocity.y = -_velocity.y;
				}
			}
		}			
	}
}

bool enemy_t::on_interact(player_t *player)
{
	if (_distance <_diameter + player->radius()){
		player->change_health(-_damage);
		return true;
	}
	return false;
}

bool enemy_t::collide(game_object_t *obj)
{
	_health -= obj->damage();
	obj->remove();
	if (_health <= 0){
		remove();
		return true;
	}
	return false;
}


zombie_t::zombie_t(texture_t *texture, vector_float_t position, vector_float_t velocity):
	enemy_t(texture, position, velocity)
{
	_max_health = 80;
	_health = 80;
	_damage = 2;
	_diameter = 0.2;
}

int zombie_t::update(map_t *map, list_t *objects, player_t* player)
{
	int8_t move_result = map_move(map);
	if (move_result == -1){
		return 0;
	}
	enemy_t::collide_dynamic_objects(objects);
	_timer++;
	if (_timer > 16){
		_timer = 0;
		_sprite->invert();
		const float chase_radius = 9.0f;
		const float speed = 0.03f;
		if (_distance < chase_radius && _distance > 0.1f){
			float dx = player->position().x - _position.x;
			float dy = player->position().y - _position.y;
			float inv_dist = fsqrtf(_distance);
			_velocity.x = dx * inv_dist * speed;
			_velocity.y = dy * inv_dist * speed;
		}
	}
	return 0;
}

bool zombie_t::on_interact(player_t *player)
{
	return enemy_t::on_interact(player);
}

bool zombie_t::collide(game_object_t *obj)
{
	return enemy_t::collide(obj);
}


scorpio_t::scorpio_t(texture_t *texture, vector_float_t position, vector_float_t velocity):
	enemy_t(texture, position, velocity)
{
	_max_health = 120;
	_health = 120;
	_damage = 4;
	_diameter = 0.3;
}

int scorpio_t::update(map_t *map, list_t *objects, player_t* player)
{
	int8_t move_result = map_move(map);
	if (move_result == -1){
		return 0;
	}
	enemy_t::collide_dynamic_objects(objects);
	_timer++;
	if (_timer > 16){
		_timer = 0;
		const float chase_radius = 16.0f;
		const float speed = 0.05f;
		if (_distance < chase_radius && _distance > 0.1f){
			float dx = player->position().x - _position.x;
			float dy = player->position().y - _position.y;
			float inv_dist = fsqrtf(_distance);
			_velocity.x = dx * inv_dist * speed;
			_velocity.y = dy * inv_dist * speed;
		}
	}
	return 0;
}

bool scorpio_t::on_interact(player_t *player)
{
	return enemy_t::on_interact(player);
}

bool scorpio_t::collide(game_object_t *obj)
{
	return enemy_t::collide(obj);
}


boss_t::boss_t(texture_t *texture, vector_float_t position, vector_float_t velocity):
	enemy_t(texture, position, velocity)
{
	_max_health = 800;
	_health = 800;
	_damage = 8;
	_diameter = 0.8;
	_sprite->resize(sprite_t::BIG);	
}

int boss_t::update(map_t *map, list_t *objects, player_t* player)
{
	int8_t move_result = map_move(map);
	if (move_result == -1){
		return 0;
	}
	enemy_t::collide_dynamic_objects(objects);
	_timer++;
	if (_timer > 16){
		_timer = 0;
		_sprite->invert();
		const float chase_radius = 6.0f;
		const float speed = 0.05f;
		if (_distance < chase_radius && _distance > 0.1f){
			float dx = player->position().x - _position.x;
			float dy = player->position().y - _position.y;
			float inv_dist = fsqrtf(_distance);
			_velocity.x = dx * inv_dist * speed;
			_velocity.y = dy * inv_dist * speed;
		}
	}
	return 0;
}

bool boss_t::on_interact(player_t *player)
{
	return enemy_t::on_interact(player);
}

bool boss_t::collide(game_object_t *obj)
{
	return enemy_t::collide(obj);
}


/************ static objects **************/
static_t::static_t(texture_t *texture, vector_float_t position):
	 game_object_t(texture, position, 64) {}


health_t::health_t(texture_t* texture, vector_float_t position) :
	static_t(texture, position), health_to_add(100)
{
	_diameter = 0.2f;
	_sprite->shift(32);
	_sprite->resize(sprite_t::SMALL);
}

bool health_t::on_interact(player_t *player)
{
	if (_distance < (_diameter + player->radius())){
		if (player->health() < MAX_PLAYER_HEALTH){
			player->change_health(health_to_add);
			remove();
			return true;
		} 
	}
	return false;
}


ammo_t::ammo_t(texture_t* texture, vector_float_t position) :
	static_t(texture, position), ammo_to_add(20)
{
	_diameter = 0.2f;
	_sprite->resize(sprite_t::SMALL);
}


bool ammo_t::on_interact(player_t *player)
{
	if (_distance < (_diameter + player->radius())){
		if (player->ammo() < MAX_AMMO){
			remove();
			player->add_ammo(ammo_to_add);
			return true;
		}
	}
	return false;
}


barrel_t::barrel_t(texture_t* texture, vector_float_t position) :
	static_t(texture, position)
{
	_diameter = 0.6f;
	_sprite->shift(96);
}


lamp_t::lamp_t(texture_t* texture, vector_float_t position) :
	static_t(texture, position)
{
	_diameter = 0.0f;
	_sprite->shift(-96);
	_sprite->resize(sprite_t::SMALL);
}
