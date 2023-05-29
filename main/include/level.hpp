#pragma once

#include <stdio.h>
#include "my_list.hpp"
#include "objects.hpp"
#include "map.hpp"
#include "textures.hpp"

class level_t{
public:
	map_t* map;
	list_t* objects;
	list_t* doors;
	texture_t** textures;
private:
	uint8_t curr_level;
public:
	level_t() : map(nullptr), objects(nullptr), doors(nullptr), textures(nullptr), curr_level(1) {}
	~level_t() { }
	void load(uint8_t lev, texture_t** txtrs);
	void destroy();
	uint8_t next_level() const;
};