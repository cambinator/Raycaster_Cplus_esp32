set(SOURCES "Lcd_Simple_Driver.c" "Lcd_Simple_Graphics.c"
    "fonts/comic24.c" "fonts/DefaultFont.c" "fonts/def_small.c" "fonts/Ubuntu16.c")
idf_component_register(SRCS ${SOURCES} 
                       INCLUDE_DIRS "."
                       REQUIRES driver)

