#ifndef MOUSEWHEEL_MOUSEWHEEL_H
#define MOUSEWHEEL_MOUSEWHEEL_H

#include "definitions.h"
#include "bsp/board.h"
#include "hardware/gpio.h"
#include "as5048a.h"

void init_mousewheel();
void update_mousewheel_task();

typedef struct {
    bool init;
    uint16_t last_angle;
    int16_t delta;
} MouseWheel;

#endif