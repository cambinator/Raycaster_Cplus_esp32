// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <stdio.h>
#include <stdint.h>
#include "my_list.hpp"
#include "objects.hpp"
#include "map.hpp"
#include "textures.hpp"

/* stores all objects, related to the current level */
class level_t{
public:
	map_t* map;
	list_t* objects;
	list_t* doors;
	texture_t** textures;
private:
	uint8_t curr_level;
private:
	void destroy();
public:
	level_t() : map(nullptr), objects(nullptr), doors(nullptr), textures(nullptr), curr_level(1) {}
	~level_t();
	void load(uint8_t lev, texture_t** txtrs);
	uint8_t next_level() const;				/* returns next level id */
};