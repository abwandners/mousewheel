#include "mousewheel.h"

#define MAX_VAL 16384

int16_t cyclic_diff_with_direction(uint16_t a, uint16_t b) {
    int16_t d1 = b - a;  // Direct difference
    if (d1 < 0) d1 += MAX_VAL;  // If negative, correct the wrap-around

    int16_t d2 = d1 - MAX_VAL;  // Alternative path (reverse direction)

    // Choose the shorter path
    return (d1 <= -d2) ? d1 : d2;
}

void update_mousewheel_task(MouseWheel *mouseWheel) {
    static uint32_t start_ms = 0;
    
    if (board_millis() - start_ms < INTERVAL_MS_UPDATE_MOUSEWHEEL_TASK) {
    return;
    }
    start_ms += INTERVAL_MS_UPDATE_MOUSEWHEEL_TASK;

    gpio_put(PIN_WHEEL_TASK_ACTIVE, 1);

    uint16_t current_angle = 0;
    if (as5048a_read_angle(&current_angle) == 0) {
      printf("ERROR::AS5048_read_angle\n");
    } else {
        mouseWheel->delta += cyclic_diff_with_direction(mouseWheel->last_angle, current_angle);
        
        //printf("%05u::%06d\n", current_angle, mouseWheel->delta);
        mouseWheel->last_angle = current_angle;
    }
    
    gpio_put(PIN_WHEEL_TASK_ACTIVE, 0);
}

void init_mousewheel(MouseWheel *mouseWheel) {
  uint16_t initial_angle = 0;
  if (as5048a_read_angle(&initial_angle) == 0) {
    printf("ERROR::AS5048_read_angle\n");
  } else {
    mouseWheel->last_angle = initial_angle;
    mouseWheel->delta = 0;
    mouseWheel->init = true;

    //printf("initial_angle: %u\n", initial_angle);
    //printf("last_angle: %u\n", mouseWheel->last_angle);
    //printf("delta: %d\n", mouseWheel->delta);
    //printf("init: %u\n", mouseWheel->init);
  }
}

