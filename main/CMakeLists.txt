set(SOURCES "main.cpp"
	"source/map.cpp" "source/doors.cpp" "source/textures.cpp"  "source/sprite.cpp"
        "source/images.cpp" "source/player.cpp" "source/objects.cpp" 
        "source/level.cpp" "source/game_play.cpp" "source/menu.cpp"  
        "source/ray.cpp" "source/raycaster.cpp"
        "maps/map_1.c" "maps/map_2.c" "maps/map_3.c" "maps/map_4.c" "maps/map_5.c")
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "." "include/")
