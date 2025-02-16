#include "usb.h"

void tud_mounted_cb() {
    printf("tud_mounted_cb\n");
}

void tud_unmounted_cb() {
    printf("tud_unmounted_cb\n");
}

void tud_suspend_cb(bool remote_wakeup_en) {
    printf("tud_suspend_cb\n");
}

void tud_resume_cb() {
    printf("tud_resume_cb\n");
}

void send_hid_report(uint8_t report_id, MouseWheel *mouseWheel) {
    //printf("send_hid_report\n");
    if (!tud_hid_ready()) {
        return;
    }

    //printf("send_hid_report:tud_hid_ready\n");

    switch (report_id) {
    case REPORT_ID_MOUSE:
        int8_t delta = mouseWheel->delta/100;
        if (delta > 0 || delta < 0) {
            printf("delta: %d\n", delta);
            mouseWheel->delta = 0;
        }
        // get data from scrollwheel
        tud_hid_mouse_report(REPORT_ID_MOUSE, delta, 0, 0, 0, 0);
        break;
    
    default:
        printf("send_hid_report::unknown report_id: %u\n", report_id);
        break;
    }

}

void hid_task(MouseWheel *mouseWheel) {
    static uint32_t start_ms = 0;
    if ( board_millis() - start_ms < INTERVAL_MS_HID_TASK){
        return;
    }
    start_ms += INTERVAL_MS_HID_TASK;
    gpio_put(PIN_HID_TASK_ACTIVE, 1);

    if (!tud_suspended()) {
        send_hid_report(REPORT_ID_MOUSE, mouseWheel);
    }

    gpio_put(PIN_HID_TASK_ACTIVE, 0);
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    printf("tub_hid_report_complete_cb\n");
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    printf("tud_hid_get_report_cb\n");
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    printf("tud_hid_set_report_cb\n");
}

