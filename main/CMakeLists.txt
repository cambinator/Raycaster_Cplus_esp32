set(SOURCES "main.cpp"
	    "sources/map.cpp" "sources/doors.cpp" "sources/textures.cpp" "sources/sprite.cpp"
        "sources/images.cpp" "sources/player.cpp" "sources/objects.cpp" 
        "sources/level.cpp" "sources/game_play.cpp" "sources/menu.cpp"  
        "sources/ray.cpp" "sources/raycaster.cpp"
        "maps/map_1.c" "maps/map_2.c" "maps/map_3.c" "maps/map_4.c" "maps/map_5.c")
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "." "include/")
