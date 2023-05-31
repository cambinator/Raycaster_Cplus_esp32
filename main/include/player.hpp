#pragma once
#include "types.hpp"
#include "my_list.hpp"
#include "map.hpp"
#include "textures.hpp"
#include "doors.hpp"
#include "images.hpp"
#include "objects.hpp"

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
	vector_float_t _dir_vector;
	vector_float_t _cam_vector;
	vector_float_t _position;
	const float _radius;
	const float angle_velocity;
	const float velocity;
	int16_t _health;
	uint16_t _score;
	uint8_t _height;
	uint8_t _ammo;
	uint8_t _timer;	
	eState _state;
	eAction _action;
	eWeapon _weapon;
	int8_t _pistol_shift;
public:
	image_t* images[COUNT];
private:
	bool check_objects_collision(list_t* obj_list);
	bool check_doors_open(list_t* doors);
	bool map_collision(map_t *map);
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
	uint8_t height() const
	{
		return _height;
	}
	void jump()
	{
		_height = 100;
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
