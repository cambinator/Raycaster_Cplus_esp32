#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_button.hpp"
#include "Lcd_Simple_Driver.h"
#include "Lcd_Simple_Grafics.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEBOUNCE_DELAY 50

extern button button_2;  /* move left / enter game */
extern button button_6;  /* shoot / choose game */


void Start_Screen(device_tft tft);
void End_Screen(device_tft tft, int score, const color16_t* color, const char* text);
int Start_Menu(device_tft tft);

#ifdef __cplusplus
}
#endif