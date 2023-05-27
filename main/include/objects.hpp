#pragma once

#include <math.h>
#include "esp_heap_caps.h"
#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Grafics.h"
#include "types.hpp"
#include "my_list.hpp"
//#include "raycaster.hpp"

enum eObjType : uint8_t {ZOMBIE = 1, SCORP = 2, HEALTH = 3, AMMO = 4, BARREL = 5, LAMP = 6, BOSS = 7, PORTAL = 8,
	SOLID_START = 9, SOLID_END = 14, DOOR_V = 15, DOOR_H = 16, BULLET = 17, KNIFE_S = 18, FOG = 19, DARK = 20}; 

/**************** MAPS *************************/
class map_t{
private:
	const uint8_t* _map;
	const uint16_t _width;
	const uint16_t _height;
public:
	map_t(const uint8_t* map, const uint16_t width, const uint16_t height) : 
		_map(map), _width(width), _height(height) {}
	uint16_t width() const 
	{
		return _width;
	}
	uint16_t height() const
	{
		return _height;
	}
	uint8_t get_index(uint16_t x, uint16_t y) const
	{
		if (x >= _width || y >= _height){
			return 0;
		} else {
			return _map[y * _width + x];
		}
	}
	static bool is_solid(uint8_t tile)				//little cheat to check if tile is a wall
	{
		return (tile >= SOLID_START && tile <= SOLID_END);
	}
	static bool is_door(uint8_t tile)				//little cheat to check if tile is a wall
	{
		return (tile == DOOR_H || tile == DOOR_V);
	}
};


/************ DOORS ***********/
#define DOOR_TIMER_MAX 100
#define DOOR_TIMER_MIN 8

class door_t{
public:
	enum class eState : uint8_t{OPEN, OPENING, CLOSING, CLOSED};
private:
    point_t pos;
    eState _state;
	uint8_t _timer;
public:
    door_t(uint16_t xpos, uint16_t ypos) : _state(eState::CLOSED), _timer(DOOR_TIMER_MAX)
    {
        pos.x = xpos;
        pos.y = ypos;
    }
    void open();
    void close();
    void update();
    uint8_t time() const
    {
        return _timer;
    }
    eState state() const
    { 
        return _state;
    }
    const point_t& position() const 
    {
        return pos;
    }
};


/************* TEXTURES *****************/
#define NUM_TEX 16

typedef void (*texture_t_generator_fn)(color16_t*, uint8_t, uint8_t);

class texture_t{
private:
	color16_t* _buffer;
	uint8_t _width;
	uint8_t _height;
public:
	texture_t() : _buffer(nullptr), _width(0), _height(0) {}
	~texture_t()
	{
		delete _buffer;
	}
	uint8_t width() const 
	{
		return _width;
	}
	uint8_t height() const
	{
		return _height;
	}
	const color16_t* buffer() const
	{
		return _buffer;
	}
	color16_t get_pixel(uint8_t x, uint8_t y) const;
	color16_t get_pixel_normalized(float x, float y) const;
	void create_generated(const uint8_t width, const uint8_t height, texture_t_generator_fn);
	void create_from_file(const char* filename);
	static void textures_load(texture_t* textures[], int num);
	static void Generate_Tex_1(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h);
	static void Generate_Tex_2(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h);
	static void Generate_Tex_3(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h);
};



/**************** IMAGE *********************/
class image_t{
private:
	texture_t* _texture = nullptr;
	point_t _position = {0, 0};
public:
	image_t() {}
	explicit image_t(const char* filename)
	{
		create_from_file(filename);
	}
	~image_t()
	{
		delete _texture;
	}
	const point_t& position() const
	{
		return _position;
	}
	uint8_t width() const
	{
		return _texture->width();
	}
	uint8_t height() const
	{
		return _texture->height();
	}
	void create_from_file(const char* filename);
	void move(uint16_t x, uint16_t y);
	void copy_to_buffer(color16_t* buff, uint8_t b_width, uint8_t b_height);
	void draw(device_tft tft);
};

/******************* PLAYER *********************/
constexpr int MAX_PLAYER_HEALTH = 252;
constexpr uint8_t MAX_AMMO = 200;
constexpr vector_float_t PLAYER_START_POS = {2.5f, 3.5f};
constexpr vector_float_t PLAYER_START_DIR = {-1.0f, 0.0f};
constexpr vector_float_t CAM_START_DIR = {0.0f, 0.66f};

class player_t{
public:
	enum class eState : uint8_t {GOOD, DAMAGED, DEAD};
	enum class eAction : uint8_t {STAY, RUN, ATTACK};
	enum class eDirection : uint8_t {LEFT, RIGHT};
	enum class eWeapon : uint8_t {GUN, KNIFE};
	enum eSprite : uint8_t {GUN = 0, BLOOD = 1, KNIFE1 = 2, KNIFE2 = 3, COUNT = 4};
private:
	vector_float_t _dir_vector = PLAYER_START_DIR;
	vector_float_t _cam_vector = CAM_START_DIR;
	vector_float_t _position = PLAYER_START_POS;
	const float _radius = 0.5f;
	const float angle_velocity = 0.1f;
	const float velocity = 0.1f;
	float _height = 0.0f;
	int16_t _health = MAX_PLAYER_HEALTH;
	uint16_t _score = 0;
	uint8_t _ammo = 20;
	uint8_t _timer = 0;	
	eState _state = eState::GOOD;
	eAction _action = eAction::STAY;
	eWeapon _weapon = eWeapon::GUN;
	int8_t _pistol_shift = 1;
public:
	image_t* images[COUNT];
public:
	player_t();
	~player_t();
	const vector_float_t& direction() const
	{
		return _dir_vector;
	}
	const vector_float_t& camera_vec() const
	{
		return _cam_vector;
	}
	const vector_float_t& position() const
	{
		return _position;
	}
	float radius() const
	{
		return _radius;
	}
	float height() const
	{
		return _height;
	}
	void jump()
	{
		_height = 100.0f;
	}
	uint16_t score() const
	{
		return _score;
	}
	void gain_score(uint16_t points)
	{
		_score += points;
	}
	uint8_t ammo() const
	{
		return _ammo;
	}
	int16_t health() const
	{
		return _health;
	}
	eState state() const
	{
		return _state;
	}
	eAction action() const
	{
		return _action;
	}
	eWeapon weapon() const
	{
		return _weapon;
	}
	void set_weapon(eWeapon wpn);
	void add_ammo(uint8_t bullets);
	void gun_swing();
	void update();
	int change_health(int level);
	uint8_t move_forward(map_t* map, list_t* obj_list, list_t* doors);
	uint8_t move_side(map_t* map, list_t* obj_list, eDirection dir);
	void turn(eDirection dir);
	void shoot(list_t* obj_list, texture_t* textures[]);
	void knife_attack(list_t* obj_list);
	void attack(list_t* obj_list, texture_t* textures[]);
};

/****************** OBJECTS ********************/
class game_object_t{
	friend class raycaster_t;
protected:
	texture_t* texture = nullptr;
	vector_float_t _position = {0.0f, 0.0f};
	float _distance = INFINITY_;
	float _diameter = 0.0f;
	int16_t _max_health = 2000;
	int16_t _health = 2000;
	eObjType _type;
	bool _to_remove = false;
	bool _transparent = false;
	bool _inverted = false;
	uint8_t _damage = 0;
	bool _friendly = true;
	int8_t vert_shift = 0;
	uint8_t size_divider = 2;
public:
	game_object_t() {}
	game_object_t(texture_t* texture, vector_float_t position, int vert_shift, eObjType type) : 
		texture(texture), _position(position), _type(type), vert_shift(vert_shift) {}
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
	bool inverted() const
	{
		return _inverted;
	}
	bool transparent() const
	{
		return _transparent;
	}
	eObjType type() const
	{
		return _type;
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
	void update_distance(player_t* player);
	static bool compare_distances(void* a, void* b);
	static void clean_objects_list(list_t* objects_list);
	virtual int update(map_t* map, list_t* objects, player_t* player) = 0;
	virtual bool on_interact(player_t* player) = 0;
	virtual bool collide(game_object_t* obj) = 0;
};

/**** dynamic objects *****/

class dynamic_t : public game_object_t{	
private:
	vector_float_t _velocity; 
	uint8_t _timer = 0;
public:
	dynamic_t(texture_t* texture, vector_float_t position, vector_float_t velocity, int v_shift, eObjType type);
	~dynamic_t(){}
	int update(map_t* map, list_t* objects, player_t* player);
	bool on_interact(player_t* player) override;
	bool collide(game_object_t* obj);
};

/******* static objects **********/
class static_t : public game_object_t{
public: 
	static_t(texture_t* texture, vector_float_t position, eObjType type);
	~static_t(){}
	int update(map_t* map, list_t* objects, player_t* player) { return 0; }
	bool on_interact(player_t* player) override;
	bool collide(game_object_t* obj) { return false;}
};


/******** GAME SCENE ******/
void game_scene_update(player_t* player, map_t* map, list_t *objects, list_t* doors);