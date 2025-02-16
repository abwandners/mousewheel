#ifndef AS5048A_H
#define AS5048A_H

#include "pico/stdlib.h"
#include "hardware/spi.h"

#define AS5048A_OK           0x00
#define AS5048A_ERROR_PARITY 0x01
#define AS5048A_ERROR_FLAG   0x02

void init_as5048a(spi_inst_t *spi, uint cs_pin);

uint8_t as5048a_read_angle(uint16_t *angle);
uint8_t as5048a_clear_error_flag(uint16_t *error_flag);
uint8_t as5048a_read_diagnostics_automatic_gain_control_agc(uint16_t *automatic_gain_control_agc);
uint8_t as5048a_read_magnitude(uint16_t *magnitude);
uint8_t as5048a_read_angle(uint16_t *angle);

#endif