set(SOURCES "Lcd_Simple_Driver.c" "Lcd_Simple_Graphics.c"
    "comic24.c" "DefaultFont.c" "def_small.c" "Ubuntu16.c")
idf_component_register(SRCS ${SOURCES} 
                       INCLUDE_DIRS "."
                       REQUIRES driver)

