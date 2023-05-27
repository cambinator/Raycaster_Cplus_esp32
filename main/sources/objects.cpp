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


/*********** DOORS ***************/
void door_t::open(){
	if (_state == eState::CLOSED){
		_state = eState::OPENING;
		_timer = DOOR_TIMER_MAX;
	}
}

void door_t::close(){
	if (_state == eState::OPEN){
	    _state = eState::CLOSING;
		_timer = DOOR_TIMER_MIN;
	}
}

void door_t::update()
{
	if (_state == eState::OPENING){
	 	if (_timer > DOOR_TIMER_MIN){
			_timer -= 1;
		} else {
			_state = eState::OPEN;
		}
	}
	if (_state == eState::CLOSING){
	 	if (_timer < DOOR_TIMER_MAX){
			_timer += 1;
		} else {
			_state = eState::CLOSED;
		}
	}
}


/****************** TEXTURES **************/
void texture_t::create_from_file(const char *fname)
{
	int bytes_read = 0;
	char img_size[2];
	FILE* file = nullptr;
	if (fname){
		file = fopen(fname, "r");
		if (file == nullptr){
			printf("texture_t::create: cannot open file: %s\n", fname);
			return;
		}
		bytes_read = fread(img_size, 1, 2, file);
		if (bytes_read != 2){
			printf("texture_t::create: empty file %s\n", fname);
		}
		//printf("file %s sizes - width %u, height %u\n", fname, img_size[0], img_size[1]); /* debug */
		_width = img_size[0];
		_height = img_size[1];
		uint32_t file_size = _width * _height * sizeof(color16_t);
		_buffer = (color16_t*)heap_caps_malloc(file_size, MALLOC_CAP_DMA);
		bytes_read = fread(_buffer, 1, file_size, file);
		if (bytes_read != file_size){
			printf("texture_t::create: wrong file or image side size - %d not %lu\n", bytes_read, file_size);
			fclose(file);
			return;
		}
	} else {
		puts("texture_t::create: empty filename");
		return;
	}
	fclose(file);
}

color16_t texture_t::get_pixel(uint8_t x, uint8_t y) const
{
	return _buffer[y * _width + x];
}

color16_t texture_t::get_pixel_normalized(float xnorm, float ynorm) const
{
	uint8_t x = (uint8_t)(xnorm * _width);
	uint8_t y = (uint8_t)(ynorm * _height);
	return _buffer[y * _width + x];
}

void texture_t::create_generated(const uint8_t width, const uint8_t height, texture_t_generator_fn func)
{
	_width = width;
	_height = height;
	_buffer = (color16_t*)heap_caps_malloc(height * width * PIXEL_SIZE, MALLOC_CAP_DMA);
	if (_buffer == nullptr) {
		puts("texture_t: cannot allocate memory");
		return;
	}
	func(_buffer, width, height);
	return; 
}

void texture_t::textures_load(texture_t *textures[], int num)
{
	for (int i = 0; i != num; i++){
		textures[i] = new texture_t;
	}	
	/* solid objects	 */
	textures[0]->create_from_file("/spiffs/onyx_16b.bin");
	textures[1]->create_from_file("/spiffs/concrete_16b.bin");
	textures[2]->create_from_file("/spiffs/iron_16b.bin");
	textures[3]->create_from_file("/spiffs/stone2_16b.bin");
	textures[4]->create_from_file("/spiffs/stone1_16b.bin");
	textures[5]->create_from_file("/spiffs/brick2_16b.bin");
	textures[6]->create_from_file("/spiffs/wood_16b.bin");
	textures[7]->create_from_file("/spiffs/door_16b.bin");
	/* dynamic objects */
	textures[8]->create_from_file("/spiffs/zombie_16b.bin");
	textures[9]->create_from_file("/spiffs/scorpion_16b.bin");
	textures[10]->create_from_file("/spiffs/heart_16b.bin");
	textures[11]->create_from_file("/spiffs/fireball_16b.bin");
	textures[12]->create_from_file("/spiffs/ammo_16b.bin");
	textures[13]->create_from_file("/spiffs/barrel_16b.bin");
	textures[14]->create_from_file("/spiffs/lamp1_16b.bin");
	textures[15]->create_from_file("/spiffs/burer64_16b.bin");
}

/* red bricks */
void texture_t::Generate_Tex_1(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h)
{
	for (int j = 0; j != tex_h; j++){
		for (int i = 0; i != tex_w; i++){
			color16_t color = cRED;
			color = color * (i % 8 && j % 8);
			tex[j * tex_w + i] = color;
		}
	}
}

/* y gradient */
void texture_t::Generate_Tex_2(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h)
{
	for (int j = 0; j != tex_h; j++){
		for (int i = 0; i != tex_w; i++){
			color16_t color = cBLUE;
			color = (color << 8)|(color >> 8);
			color = ((j * color) / tex_h);
			color = (color << 8)|(color >> 8);
			tex[j * tex_w + i] = color;
		}
	}
}

/* x gradient */
void texture_t::Generate_Tex_3(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h)
{
	for (int j = 0; j != tex_h; j++){
		for (int i = 0; i != tex_w; i++){
			color16_t color = cBLUE;
			color = (color << 8)|(color >> 8);
			color = ((i * color) / tex_h);
			color = (color << 8)|(color >> 8);
			tex[j * tex_w + i] = color;
		}
	}
}

/*************** IMAGE ****************/
void image_t::create_from_file(const char *filename)
{
	_texture = new texture_t;
	_texture->create_from_file(filename);
}
void image_t::move(uint16_t x, uint16_t y)
{
	_position.x = x;
	_position.y = y;
}

void image_t::copy_to_buffer(color16_t* buff, uint8_t b_width, uint8_t b_height)
{
	if (b_width < width() || b_height < height()){
		puts("Sprite_Copy_To_Buffer: image is too big");
		return;
	}
	uint32_t row_width = width();
	uint32_t buf_ind = (_position.y * b_width + _position.x);		
	for (uint8_t j = 0; j != height(); ++j){
		row_width = width();
		for (uint8_t i = 0; i != width(); ++i) {		
			color16_t color = _texture->get_pixel(i, j);
			if (!iCompare_Colors(&color, &cBLACK)){
				buff[buf_ind] = _texture->get_pixel(i, j);
			}
			buf_ind ++;
			if ((_position.x + i) >= b_width){
				row_width = _position.x + i;
				break;
			}
		}
		buf_ind += (b_width - row_width);
		if (_position.y + j >= b_height){
			break;
		}
	}
}

void image_t::draw(device_tft tft)
{
	if (tft == nullptr || _texture == nullptr){
		return;	
	}
	vSend_Frame(tft, _position.x, _position.y, _texture->width(), _texture->height(), (uint8_t*)_texture->buffer()); 
}


/****************** PLAYER *******************/
player_t::player_t()
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
		_height -= 10.0f;
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
uint8_t player_t::move_forward(map_t* map, list_t* objects, list_t* doors)
{
	/* function checks independently movement in x and y direction to produce nicer, gliding movement around obstacles */
	int current_tile = map->get_index((uint16_t)_position.x, (uint16_t)_position.y);
	float dx = _dir_vector.x * velocity;
	float dy = _dir_vector.y * velocity;
	
	/* move in x direction */
	_position.x += dx;       
	current_tile = map->get_index((uint16_t)_position.x, (uint16_t)_position.y);	
	/* check if on door tile */
	uint8_t door_opened = 0;	
	for (auto iter = doors->begin(); iter != doors->end(); ++iter){
		door_t* door = (door_t*)*iter;
		if (door->position().x == (uint16_t)_position.x && door->position().y == (uint16_t)_position.y){
			if (door->state() == door_t::eState::OPEN){
				door_opened = 1;
			} else {
				door->open();
			}
			break;
		}
	}		
	if (map_t::is_solid(current_tile) || (map_t::is_door(current_tile) && !door_opened)){
		_position.x -= dx;
	}	
	/* check for solid objects (solid objects have obj->width > 0) */
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		if (obj->diameter() > 0.0f && obj->distance() < 2.0f){		
			obj->update_distance(this);
			if (obj->distance() < obj->diameter()){
				_position.x -= dx;
			}
		}
	}	

	/* move in y direction */
	_position.y += dy;  	
	current_tile = map->get_index((uint16_t)_position.x, (uint16_t)_position.y);
	/* check if on door tile */
	door_opened = 0;	
	for (auto iter = doors->begin(); iter != doors->end(); ++iter){
		door_t* door = (door_t*)*iter;
		if (door->position().x == (uint16_t)_position.x && door->position().y == (uint16_t)_position.y){
			if (door->state() == door_t::eState::OPEN){
				door_opened = 1;
			} else {
				door->open();
			}
			break;
		}
	}	
	if (map_t::is_solid(current_tile) || (map_t::is_door(current_tile) && !door_opened)){
		_position.y -= dy;
	}	
    /* check for solid objects (solid objects have obj->width > 0) */
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		if (obj->diameter() > 0.0f && obj->distance() < 2.0f){		
			obj->update_distance(this);
			if (obj->distance() < obj->diameter()){
				_position.y -= dy;
			}
		}
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
	if (map_t::is_solid(current_tile) || map_t::is_door(current_tile)){
		_position.x += dx;
		_position.y -= dy;
	}	
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		if (obj->diameter() > 0.0f && obj->distance() < 2.0f){		
			obj->update_distance(this);
			if (obj->distance() < obj->diameter()){
				_position.x += dx;
				_position.y -= dy;
			}
		}
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
		dynamic_t* bullet = new dynamic_t(textures[11], {_position.x + _dir_vector.x, 
			_position.y + _dir_vector.y}, _dir_vector, 64 - _height, BULLET);
		obj_list->push_back((void*)bullet);	
	}
}
void player_t::knife_attack(list_t *obj_list)
{
	dynamic_t* knife_sphere = new dynamic_t(nullptr, _position, {0, 0}, 0, KNIFE_S);
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

/****************** OBJECTS ********************/
bool game_object_t::compare_distances(void *a, void *b)
{
    game_object_t* first = (game_object_t*)a;
	game_object_t* second = (game_object_t*)b;
	return (first->distance() < second->distance());
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


/************ dynamic objects **************/
dynamic_t::dynamic_t(texture_t *texture, vector_float_t position, vector_float_t velocity, int v_shift, eObjType type):
	 game_object_t(texture, position, v_shift, type), _velocity(velocity)
{
	switch(type){
		case SCORP:
			_max_health = 120;
			_health = 120;
			_damage = 4;
			_diameter = 0.3;
			vert_shift = 64;
			_friendly = false;
			break;
		case ZOMBIE:
			_max_health = 80;
			_health = 80;
			_damage = 2;
			_diameter = 0.2;
			vert_shift = 64;
			_friendly = false;
			break;
		case BULLET:
			_damage = 40;
			_diameter = 0.0f;
			break;
		case KNIFE_S:
			_damage = 60;
			_diameter = 0.0f;
			break;
		case BOSS:
			_max_health = 800;
			_health = 800;
			_damage = 8;
			_diameter = 0.8;
			vert_shift = 64;
			size_divider = 1;
			_friendly = false;
			break;
		default:
			break;
	}
}

int dynamic_t::update(map_t *map, list_t *objects, player_t* player)
{
	if (_to_remove){
		return 0;
	}
	/* move x */
	_position.x += _velocity.x;
	int obj_x_h = floorf(_position.x + _diameter);  
	int obj_x_l = floorf(_position.x- _diameter);
	int obj_y_h = floorf(_position.y + _diameter);
	int obj_y_l = floorf(_position.y - _diameter);
	if (obj_x_l < 0 || obj_x_h >= map->width()){
		_to_remove = 1;
		return 0;
	}
	if (map_t::is_solid(map->get_index(obj_x_h, obj_y_h)) || map_t::is_solid(map->get_index(obj_x_l, obj_y_l)) || 
		map_t::is_solid(map->get_index(obj_x_h, obj_y_l)) || map_t::is_solid(map->get_index(obj_x_l, obj_y_h))){
		if (_type == BULLET){
			_to_remove = 1;
			return 0;
		} else {
			_velocity.x = -_velocity.x;
			_position.x += _velocity.x;
			if (_type == SCORP){
				_inverted = !_inverted;
			}
		}
	}	
	/* move y */
	_position.y += _velocity.y;
	obj_y_h = floorf(_position.y + _diameter);
	obj_y_l = floorf(_position.y - _diameter);
	if (obj_y_l < 0 || obj_y_h >= map->height()){
		_to_remove = 1;
		return 0;
	}
	if (map_t::is_solid(map->get_index(obj_x_h, obj_y_h)) || map_t::is_solid(map->get_index(obj_x_l, obj_y_l)) || 
		map_t::is_solid(map->get_index(obj_x_h, obj_y_l)) || map_t::is_solid(map->get_index(obj_x_l, obj_y_h))){
		if (_type == BULLET){
			_to_remove = 1;
			return 0;
		} else {
			_velocity.y = -_velocity.y;
			_position.y += _velocity.y;
			//_inverted = !_inverted;
		}
	}
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* next_object = (game_object_t*)*iter;
		if (next_object != (game_object_t*)this){			
			float x = fabs(_position.x - next_object->position().x);
			float y = fabs(_position.y - next_object->position().y);
			float radius = _diameter + next_object->diameter();
			if (x <= radius * 2 && y <= radius * 2){
				if ((_type == BULLET || _type == KNIFE_S)){
					if  (!_to_remove){
						if (next_object->collide(this)){
							return 1;
						}
					}
				} else if (next_object->diameter() > 0.0f){
					_velocity.x = -_velocity.x;
					_velocity.y = -_velocity.y;
				}
			}
		}			
	}
	_timer++;
	if (_timer > 16 && _type == ZOMBIE){
		_timer = 0;
		_inverted = !_inverted;
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
	if (_timer > 16 && _type == SCORP){
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
	if (_timer > 16 && _type == BOSS){
		_timer = 0;
		_inverted = !_inverted;
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
	if (_timer > 4 && _type == KNIFE_S){
		_timer = 0;
		remove();
	}
	return 0;
}

bool dynamic_t::on_interact(player_t *player)
{
	if (!_friendly){
		if (_distance <_diameter + player->radius()){
			player->change_health(-_damage);
			return true;
		}
	}
	return false;
}

bool dynamic_t::collide(game_object_t *obj)
{
	if ((_type != BULLET) ){
		_health -= obj->damage();
		obj->remove();
		if (_health <= 0){
			remove();
			return true;
		}
	}
	return false;
}


/************ static objects **************/
static_t::static_t(texture_t *texture, vector_float_t position, eObjType type):
	 game_object_t(texture, position, 64, type)
{
	switch(type){
		case HEALTH:
			_diameter = 0.2f;
			vert_shift = 32;
			size_divider = 4;
			break;
		case AMMO:
			_diameter = 0.2f;
			vert_shift = 64;
			size_divider = 4;
			break;
		case BARREL:
			_diameter = 0.6f;
			vert_shift = 96;  
			size_divider = 2;
			break;
		case LAMP:
			_diameter = 0.0f;
			vert_shift = -96;				/* shift up */
			size_divider = 4;
			break;
		default:
			_diameter = 0.0f;
			vert_shift = 64;
			size_divider = 2;
			break;
	}
}

bool static_t::on_interact(player_t *player)
{
	if (_diameter > 0.0f && _distance < (_diameter + player->radius())){
		if (_type == HEALTH && player->health() < MAX_PLAYER_HEALTH){
			player->change_health(100);
			remove();
			return true;
		} 
		if (_type == AMMO && player->ammo() < MAX_AMMO){
			remove();
			player->add_ammo(20);
			return true;
		}
	}
	return false;
}


/******** GAME SCENE ******/
void game_scene_update(player_t* player, map_t* map, list_t *objects, list_t* doors)
{
	player->update();
	//distance update is handled in raycaster and move func
	bool check_enemy_near = false;
	for (auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* obj = (game_object_t*)*iter;
		obj->update_distance(player);
		obj->on_interact(player);
		if (obj->update(map, objects, player)){
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

	for (auto iter = doors->begin(); iter != doors->end(); ++iter){
		door_t* door = (door_t*)*iter;
		door->update();
	}	
}
	
