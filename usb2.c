#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "usb.h"

#include "mousewheel.h"


/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static uint32_t btn_read_interval_ms = 100;

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, MouseWheel *mouseWheel) {
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id) {
    case REPORT_ID_MOUSE:
      int8_t delta = mouseWheel->delta/100;
      if (delta > 0 || delta < 0) {
        mouseWheel->delta = 0;
      }
      //if (btn_pressed) {
      // no button, no right, no down, scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 0, 0, delta, 0);
      //  printf("btn: %u\n", btn_pressed);
      //} else {
        // no button, no right, no down, no scroll, no pan
        //tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
      //  tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 0, 0, 0, 0);
        //printf("btn: %u\n", btn_pressed);
      
      //btn_pressed = false;
      break;
    
    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(MouseWheel *mouseWheel) {
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;
  gpio_put(PIN_HID_TASK_ACTIVE, 1);
  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn ) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  } else {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, mouseWheel);
  }
  gpio_put(PIN_HID_TASK_ACTIVE, 0);
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    //send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  printf("tud_hid_get_report_cb");
  // TODO not Implemented
  /*
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;
  */

  if (report_type == HID_REPORT_TYPE_FEATURE) {
        // Return the current resolution multiplier
        //buffer[0] = resolution_multiplier;
        //return 1; // 1-byte report
    }


  return 0; // Unsupported report type

}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;
  printf("HID_REPORT_TYPE_FEATURE");

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
  if (report_type == HID_REPORT_TYPE_FEATURE) {
        //resolution_multiplier = buffer[0]; // Assuming the multiplier is in the first byte

        // Ensure it's within a valid range
        //if (resolution_multiplier < 1) resolution_multiplier = 1;
        //if (resolution_multiplier > 16) resolution_multiplier = 16;

        //printf("resultion multiplier set to %hhu", resolution_multiplier);
    }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;
  

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;
  gpio_put(PIN_LED_BLINKING_TASK_ACTIVE, 1);

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
  gpio_put(PIN_LED_BLINKING_TASK_ACTIVE, 0);
}
