/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "class/hid/hid.h"



/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

// Define additional HID constants
#define HID_USAGE_PAGE_DIGITIZER 0x0D
#define HID_USAGE_DIGITIZER_RESOLUTION_MULTIPLIER 0x48
#define HID_END_COLLECTION 0xC0

#define MICROSOFT_OS_DESCRIPTORS 0xEE


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
/*
uint8_t const desc_hid_report[] = {
    // Usage Page (Generic Desktop)
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),  
    // Usage (Mouse)
    HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE    ),  
    // Collection (Application)
    HID_COLLECTION ( HID_COLLECTION_APPLICATION ),  

        // Wheel Input
        HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP                 ),  // Generic Desktop
        HID_USAGE          ( HID_USAGE_DESKTOP_WHEEL                ),  // Wheel
        HID_LOGICAL_MIN    ( 0                                      ),  // Logical Min (0)
        HID_LOGICAL_MAX    ( 1                                      ),  // Logical Max (1)
        HID_PHYSICAL_MIN_N ( -127, 2                                ),  // Physical Min (-127)
        HID_PHYSICAL_MAX_N ( 127, 2                                 ),  // Physical Max (127)
        HID_REPORT_SIZE    ( 1                                      ),  // Report Size (1 bit)
        HID_REPORT_COUNT   ( 1                                      ),  // Report Count (1)
        HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), // Input (Data, Var, Abs)

        // Resolution Multiplier Feature Report (must be defined manually)
        HID_USAGE_PAGE     ( HID_USAGE_PAGE_ORDINAL                 ),  // Ordinal Page (custom usage)
        HID_USAGE          ( 0x48                                   ),  // Resolution Multiplier (custom)
        HID_LOGICAL_MIN    ( 0                                      ),  // Logical Min (0)
        HID_LOGICAL_MAX    ( 1                                      ),  // Logical Max (1)
        HID_PHYSICAL_MIN   ( 1                                      ),  // Physical Min (1)
        HID_PHYSICAL_MAX   ( 255                                    ),  // Physical Max (255)
        HID_REPORT_SIZE    ( 8                                      ),  // Report Size (8-bit)
        HID_REPORT_COUNT   ( 1                                      ),  // Report Count (1)
        HID_FEATURE        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), // Feature Report

    HID_COLLECTION_END // End Collection
};
*/
uint8_t const desc_hid_report3[] = {
  TUD_HID_REPORT_DESC_MOUSE   ( HID_REPORT_ID(REPORT_ID_MOUSE            )),
};

uint8_t const desc_hid_report2[] = {
  HID_REPORT_ID(REPORT_ID_MOUSE)
  HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
  HID_USAGE(HID_USAGE_DESKTOP_MOUSE),
  HID_COLLECTION(HID_COLLECTION_APPLICATION),
    HID_USAGE(HID_USAGE_DESKTOP_POINTER),
    HID_COLLECTION (HID_COLLECTION_PHYSICAL),

      HID_USAGE(HID_USAGE_DESKTOP_RESOLUTION_MULTIPLIER),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(1),
      HID_PHYSICAL_MIN(1),
      HID_PHYSICAL_MAX(120),
      HID_REPORT_SIZE(8),
      HID_REPORT_COUNT(1),
      HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      HID_USAGE(HID_USAGE_DESKTOP_WHEEL),
      HID_LOGICAL_MIN(-127),
      HID_LOGICAL_MAX(127),
      HID_REPORT_COUNT(1),
      HID_REPORT_SIZE(8),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE),

    HID_COLLECTION_END,
  HID_COLLECTION_END

};

uint8_t const desc_hid_report[] = {
    HID_REPORT_ID(REPORT_ID_MOUSE)
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),          // Usage Page (Generic Desktop)
    HID_USAGE(HID_USAGE_DESKTOP_MOUSE),             // Usage (Mouse)
    HID_COLLECTION(HID_COLLECTION_APPLICATION),     // Collection (Application)
      HID_USAGE(HID_USAGE_DESKTOP_POINTER),
      HID_COLLECTION (HID_COLLECTION_PHYSICAL),
          
        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
          HID_USAGE_MIN(1),
          HID_USAGE_MAX(5),
          HID_LOGICAL_MIN(0),
          HID_LOGICAL_MAX(1),
          /* Left, Right, Middle, Backward, Forward buttons */
          HID_REPORT_COUNT(5),
          HID_REPORT_SIZE(1),
          HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
          /* 3 bit padding */
          HID_REPORT_COUNT(1),
          HID_REPORT_SIZE(3),
          HID_INPUT(HID_CONSTANT),

        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
          HID_USAGE(HID_USAGE_DESKTOP_X),
          HID_USAGE(HID_USAGE_DESKTOP_Y),
          HID_LOGICAL_MIN(-127),
          HID_LOGICAL_MAX(127),
          HID_REPORT_COUNT(2),
          HID_REPORT_SIZE(8),
          HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE),
          
          HID_USAGE(HID_USAGE_DESKTOP_RESOLUTION_MULTIPLIER),
          HID_LOGICAL_MIN(0),
          HID_LOGICAL_MAX(1),
          HID_PHYSICAL_MIN(1),
          HID_PHYSICAL_MAX(120),
          HID_REPORT_SIZE(4),
          HID_REPORT_COUNT(1),
          HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
          
          HID_USAGE(HID_USAGE_DESKTOP_WHEEL),
          HID_LOGICAL_MIN(-127),
          HID_LOGICAL_MAX(127),
          HID_REPORT_COUNT(1),
          HID_REPORT_SIZE(8),
          HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE),
      HID_COLLECTION_END,
    HID_COLLECTION_END
};




// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
   printf("tud_hid_descriptor_report_cb\n");
  return desc_hid_report2;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID   0x81

uint8_t const desc_configuration[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
  TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report2), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

  // This example use the same configuration for both high and full speed mode
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// String Descriptor Index
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

// array of pointer to string descriptors
char const *string_desc_arr[] =
{
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "TinyUSB",                     // 1: Manufacturer
  "TinyUSB Device",              // 2: Product
  NULL,                          // 3: Serials will use unique ID if possible
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  printf("%hhu\n", index);
  (void) langid;
  size_t chr_count;

  switch ( index ) {
    case STRID_LANGID:
      printf("STRID_LANGID\n");
      memcpy(&_desc_str[1], string_desc_arr[0], 2);
      //_desc_str[1] = 0x09;
      //_desc_str[2] = 0x04;
      chr_count = 1;
      break;

    case STRID_SERIAL:
      printf("STRID_SERIAL\n");
      chr_count = board_usb_get_serial(_desc_str + 1, 32);
      break;
    //case MICROSOFT_OS_DESCRIPTORS:
    //    printf("MICROSOFT_OS_DESCRIPTORS\n");
    //    break;

    default:
      // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
      // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors
      printf("default %hhu\n", index);
      if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) return NULL;

      const char *str = string_desc_arr[index];

      // Cap at max char
      chr_count = strlen(str);
      size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
      if ( chr_count > max_count ) chr_count = max_count;

      // Convert ASCII string into UTF-16
      for ( size_t i = 0; i < chr_count; i++ ) {
        _desc_str[1 + i] = str[i];
      }
      break;
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}