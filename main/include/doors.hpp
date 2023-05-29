#pragma once

#include "types.hpp"
#include "Lcd_Simple_Graphics.h"

/************ DOORS ***********/
#define DOOR_TIMER_MAX 100
#define DOOR_TIMER_MIN 8

class door_t{
public:
	enum class eState : uint8_t{OPEN, OPENING, CLOSING, CLOSED};
private:
    point_t _position;
    eState _state;
	uint8_t _timer;
public:
    door_t(point_t pos) :  _position(pos), _state(eState::CLOSED), _timer(DOOR_TIMER_MAX) {}
    void open();
    void close();
    void update();
    uint8_t time() const
    {
        return _timer;
    }
    eState state() const
    { 
        return _state;
    }
    const point_t& position() const 
    {
        return _position;
    }
};