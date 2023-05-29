#pragma once

#include <cstdint>
/* enough for game purposes */
#define INFINITY_ 1e30f

struct vector_float_t{
    float x;
    float y;
};

enum eObjType : uint8_t {NONE = 0, ZOMBIE = 1, SCORP = 2, HEALTH = 3, AMMO = 4, BARREL = 5, LAMP = 6, BOSS = 7, PORTAL = 8,
	SOLID_START = 9, SOLID_END = 14, DOOR_V = 15, DOOR_H = 16, BULLET = 17, KNIFE_S = 18, FOG = 19, DARK = 20}; 