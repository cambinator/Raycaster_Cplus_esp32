// Copyright (c) 2023 rubanyk
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "doors.hpp"

/*********** DOORS ***************/
void door_t::open(){
	if (_state == eState::CLOSED){
		_state = eState::OPENING;
		_timer = DOOR_TIMER_MAX;
	}
}

void door_t::close(){
	if (_state == eState::OPEN){
	    _state = eState::CLOSING;
		_timer = DOOR_TIMER_MIN;
	}
}

void door_t::update()
{
	if (_state == eState::OPENING){
	 	if (_timer > DOOR_TIMER_MIN){
			_timer -= 1;
		} else {
			_state = eState::OPEN;
		}
	}
	if (_state == eState::CLOSING){
	 	if (_timer < DOOR_TIMER_MAX){
			_timer += 1;
		} else {
			_state = eState::CLOSED;
		}
	}
}
