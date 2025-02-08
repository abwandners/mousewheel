#include <stdlib.h>
#include <stdio.h>
#include "bsp/board.h"
#include "as5048a.h"
#include "hardware/spi.h"
#include "definitions.h"
#include "usb.h"
#include "mousewheel.h"

static void wheel_task();

static uint32_t wheel_task_interval_ms = 1;
/*------------- MAIN -------------*/

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19



void init_spi() {
    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
}

int main(void) {
  board_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  init_spi();
  gpio_init(PIN_WHEEL_TASK_ACTIVE);
  gpio_set_dir(PIN_WHEEL_TASK_ACTIVE, GPIO_OUT);
  gpio_put(PIN_WHEEL_TASK_ACTIVE, 0);

  gpio_init(PIN_TUD_TASK_ACTIVE);
  gpio_set_dir(PIN_TUD_TASK_ACTIVE, GPIO_OUT);
  gpio_put(PIN_TUD_TASK_ACTIVE, 0);

  gpio_init(PIN_HID_TASK_ACTIVE);
  gpio_set_dir(PIN_HID_TASK_ACTIVE, GPIO_OUT);
  gpio_put(PIN_HID_TASK_ACTIVE, 0);

  gpio_init(PIN_LED_BLINKING_TASK_ACTIVE);
  gpio_set_dir(PIN_LED_BLINKING_TASK_ACTIVE, GPIO_OUT);
  gpio_put(PIN_LED_BLINKING_TASK_ACTIVE, 0);


  init_as5048a(SPI_PORT, PIN_CS);

  uint16_t as5048a_error_data;
  as5048a_clear_error_flag(&as5048a_error_data);

  MouseWheel mouseWheel;
  init_mousewheel(&mouseWheel);

  while (1)
  {
    gpio_put(PIN_TUD_TASK_ACTIVE, 1);
    tud_task(); // tinyusb device task
    gpio_put(PIN_TUD_TASK_ACTIVE, 0);
    
    hid_task(&mouseWheel);
    
    update_mousewheel_task(&mouseWheel);

    led_blinking_task();
  }
}

void wheel_task() {
  static uint32_t start_ms = 0;
  if (board_millis() - start_ms < wheel_task_interval_ms) {
    return;
  }
  gpio_put(PIN_WHEEL_TASK_ACTIVE, 1);
  start_ms += wheel_task_interval_ms;
  uint16_t as5048a_angle = 0;
  if (as5048a_read_angle(&as5048a_angle) == 0) {
    printf("%u--ERROR\n", as5048a_angle);
  } else {
    printf("%u\n", as5048a_angle);
  }
  gpio_put(PIN_WHEEL_TASK_ACTIVE, 0);
}
