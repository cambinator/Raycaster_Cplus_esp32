#include "level.hpp"

/*************** LEVEL ****************/

void level_t::load(uint8_t lev, texture_t** txtrs)
{
	curr_level = lev;
	printf("load level %u\n", curr_level);
	textures = txtrs;
	map = map_t::load_map(curr_level);
	objects = new list_t;
	doors = new list_t;
	for (uint16_t j = 0; j != map->height(); j++){
		for (uint16_t i = 0; i!= map->width(); i++){
			switch(map->get_index(i, j)){
				case ZOMBIE : {
					zombie_t* temp = new zombie_t(textures[8], {(i + 0.55f), (j + 0.45f)}, {0.03f, 0.01f});
					objects->push_back((void*)temp);
					break;
				}
				case SCORP: {
					scorpio_t* temp = new scorpio_t(textures[9], {(i + 0.55f), (j + 0.45f)}, {0.05f, 0.05f});
					objects->push_back((void*)temp);
					break;
				}
				case BOSS: {
					boss_t* temp = new boss_t(textures[15], {(i + 0.55f), (j + 0.45f)}, {0.05f, 0.05f});
					objects->push_back((void*)temp);
					break;
				}
				case HEALTH: {
					health_t* temp = new health_t(textures[10], {(i + 0.5f), (j + 0.5f)});
					objects->push_back((void*)temp);
					break;
				}
				case AMMO: {
					ammo_t* temp = new ammo_t(textures[12], {(i + 0.5f), (j + 0.5f)});
					objects->push_back((void*)temp);
					break;
				}
				case BARREL: {
					barrel_t* temp = new barrel_t(textures[13], {(i + 0.5f), (j + 0.5f)});
					objects->push_back((void*)temp);
					break;
				}
				case LAMP: {
					lamp_t* temp = new lamp_t(textures[14], {(i + 0.5f), (j + 0.5f)});
					objects->push_back((void*)temp);
					break;
				}
				case DOOR_H:
				case DOOR_V: {
					door_t* temp = new door_t({i, j});
					doors->push_back((void*)temp);
					break;
				}
				default:
					break;
			}
		}
	}	
}

void level_t::destroy()
{
	for(auto iter = objects->begin(); iter != objects->end(); ++iter){
		game_object_t* object = (game_object_t*)*iter;
		delete object;
	}
	delete objects;
	
	for(auto iter = doors->begin(); iter != doors->end(); ++iter){
		door_t* door = (door_t*)*iter;
		delete door;
	}
	delete doors;

	delete map;
}

uint8_t level_t::next_level() const
{
	if (curr_level >= NUM_MAPS){
		return 0;
	}
    return (curr_level + 1);
}
