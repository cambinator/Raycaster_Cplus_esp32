// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "simple_button.hpp"

button::button()
{
	last_click_time = 0;
	last_state = BUTTON_OFF;
	curr_click_number = 0;
	returned_value = SINGLE_CLICK;
}

void button::init(gpio_num_t button_pin, uint8_t ret_val)
{
	gpio_set_direction(button_pin, GPIO_MODE_INPUT);
	pin = button_pin;
	returned_value = (uint8_t)ret_val;
}

bool button::check()
{
	int current_state = gpio_get_level(pin);
	last_click_num = 0;
	if (current_state == BUTTON_ON && last_state == BUTTON_OFF){
		curr_click_number +=1;
		if (curr_click_number == 1){
			last_click_time = xTaskGetTickCount();
			last_click_num = 1;					/* single click */
		} else {
			TickType_t curr_time = xTaskGetTickCount();
			if ((curr_time - last_click_time) * portTICK_PERIOD_MS <= DOUBLE_CLICK_TIME){
				last_click_num = 2;				/* double click */
				curr_click_number = 0;
			} else {
				last_click_time = curr_time;
				last_click_num = 1;				/* single click */
				curr_click_number = 1;
			}
		}		
		last_state = BUTTON_ON;
	} else if (current_state == BUTTON_OFF) {
		last_state = BUTTON_OFF;
	} else {
		last_click_num = returned_value;	 /* button is still pressed. Returns true if button is sticky.  */
	}
	return last_click_num;
}
