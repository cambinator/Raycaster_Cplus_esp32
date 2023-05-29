#pragma once

#include <math.h>
#include "esp_heap_caps.h"
#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Graphics.h"
#include "types.hpp"
#include "my_list.hpp"
#include "map.hpp"
#include "textures.hpp"
#include "doors.hpp"
#include "player.hpp"
#include "sprite.hpp"

class player_t;


/*================ BASE =================*/
class game_object_t{
protected:
	sprite_t* _sprite;
	vector_float_t _position;
	float _distance;
	float _diameter;
	int16_t _max_health;
	int16_t _health;
	uint8_t _damage;
	bool _friendly;
	bool _to_remove;
public:
	game_object_t();
	game_object_t(texture_t* texture, vector_float_t position, int vert_shift);
	virtual ~game_object_t() {}
	const vector_float_t& position() const
	{
		return _position;
	}
	float distance() const
	{
		return _distance;
	}
	bool to_remove() const
	{
		return _to_remove;
	}
	float diameter() const
	{
		return _diameter;
	}
	int16_t health() const
	{
		return _health;
	}
	int16_t max_health() const
	{
		return _max_health;
	}
	bool is_friendly() const
	{
		return _friendly;
	}
	uint8_t damage() const
	{
		return _damage;
	}
	void remove()
	{
		_to_remove = true;
	}
	const sprite_t* sprite() const
	{
		return _sprite;
	}
	bool is_drawable() const
	{
		return _sprite->texture() != nullptr;
	}
	void update_distance(player_t* player);
	static bool compare_distances(void* a, void* b);
	static void clean_objects_list(list_t* objects_list);
	virtual int update(map_t* map, list_t* objects, player_t* player) = 0;
	virtual bool on_interact(player_t* player) = 0;
	virtual bool collide(game_object_t* obj) = 0;
};



/*==================== DYNAMIC OBJECTS ===========================*/

class dynamic_t : public game_object_t{	
protected:
	vector_float_t _velocity; 
	uint8_t _timer;
protected:
	int8_t map_move(map_t* map);
	int8_t map_collision(map_t* map);
public:
	dynamic_t(texture_t* texture, vector_float_t position, vector_float_t velocity, int v_shift);
	~dynamic_t()
	{
		delete _sprite;
	}
	virtual int update(map_t* map, list_t* objects, player_t* player) = 0;
	virtual bool on_interact(player_t* player) = 0;
	virtual bool collide(game_object_t* obj) = 0;
};

/******** player weapon ********/
class bullet_t : public dynamic_t{
public:
	bullet_t(texture_t* texture, vector_float_t position, vector_float_t velocity, int v_shift);
	int update(map_t* map, list_t* objects, player_t* player) override;
	bool on_interact(player_t* player) override { return false; }
	bool collide(game_object_t* obj) override { return false; }
};

class knife_t : public dynamic_t{
public:
	knife_t(texture_t* texture, vector_float_t position, vector_float_t velocity);
	int update(map_t* map, list_t* objects, player_t* player) override;
	bool on_interact(player_t* player) override { return false; }
	bool collide(game_object_t* obj) override { return false; }
};

/************** enemies ****************/
class enemy_t : public dynamic_t{
protected:
	void collide_dynamic_objects(list_t* objects);
public:
	enemy_t(texture_t* texture, vector_float_t position, vector_float_t velocity);
	virtual int update(map_t* map, list_t* objects, player_t* player) = 0;
	bool on_interact(player_t* player) override;
	bool collide(game_object_t* obj) override;
};

class zombie_t : public enemy_t{
public:
	zombie_t(texture_t* texture, vector_float_t position, vector_float_t velocity);
	int update(map_t* map, list_t* objects, player_t* player) override;
	bool on_interact(player_t* player) override;
	bool collide(game_object_t* obj) override;
};

class scorpio_t : public enemy_t{
public:
	scorpio_t(texture_t* texture, vector_float_t position, vector_float_t velocity);
	int update(map_t* map, list_t* objects, player_t* player) override;
	bool on_interact(player_t* player) override;
	bool collide(game_object_t* obj) override;
};

class boss_t : public enemy_t{
public:
	boss_t(texture_t* texture, vector_float_t position, vector_float_t velocity);
	int update(map_t* map, list_t* objects, player_t* player) override;
	bool on_interact(player_t* player) override;
	bool collide(game_object_t* obj) override;
};


/*=================== STATIC OBJECTS ===========================*/
class static_t : public game_object_t{
public: 
	static_t(texture_t* texture, vector_float_t position);
	~static_t()
	{
		delete _sprite;
	}
	virtual int update(map_t* map, list_t* objects, player_t* player) = 0;
	virtual bool on_interact(player_t* player) = 0;
	virtual bool collide(game_object_t* obj) = 0;
};

/************ loot *************/
class health_t : public static_t{
private:
	uint8_t health_to_add;
public:
	health_t(texture_t* texture, vector_float_t position);
	int update(map_t* map, list_t* objects, player_t* player) override { return 0; }
	bool on_interact(player_t* player) override;
	bool collide(game_object_t* obj) override { return false; }
};

class ammo_t : public static_t{
private:
	uint8_t ammo_to_add;
public:
	ammo_t(texture_t* texture, vector_float_t position);
	int update(map_t* map, list_t* objects, player_t* player) override { return 0; }
	bool on_interact(player_t* player) override;
	bool collide(game_object_t* obj) override { return false; }
};

/**************** non interactive ********************/
class barrel_t : public static_t{
public:
	barrel_t(texture_t* texture, vector_float_t position);
	int update(map_t* map, list_t* objects, player_t* player) override { return 0; }
	bool on_interact(player_t* player) override { return false; }
	bool collide(game_object_t* obj) override { return false; }
};

class lamp_t : public static_t{
public:
	lamp_t(texture_t* texture, vector_float_t position);
	int update(map_t* map, list_t* objects, player_t* player) override { return 0; }
	bool on_interact(player_t* player) override { return false; }
	bool collide(game_object_t* obj) override { return false; }
};