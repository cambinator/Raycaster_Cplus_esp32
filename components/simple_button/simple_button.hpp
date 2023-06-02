// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef SIMPLE_BUTTON
#define SIMPLE_BUTTON

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BUTTON_ON 1
#define BUTTON_OFF 0

#define DOUBLE_CLICK_TIME 200

#define SINGLE_CLICK 0
#define STICKING_KEY 1

#ifdef __cplusplus
extern "C" {
#endif
class button{
	TickType_t last_click_time;
	gpio_num_t pin;
	uint8_t last_state;
	uint8_t curr_click_number;	
	uint8_t returned_value;
	uint8_t last_click_num;

	public:
	button();
	void init(gpio_num_t pin, uint8_t ret_val);
	bool check();
	uint8_t last_click_number() const
	{
		return last_click_num;
	}
};


#ifdef __cplusplus
}
#endif

#endif