#include "as5048a.h"

#define AS5048A_NOP 0x0000

#define AS5048A_CMD_ANGLE 0x3FFF
#define AS5048A_CMD_CLEAR_ERROR_FLAG 0x0001
#define AS5048A_CMD_DIAGNOSTICS_AUTOMATIC_GAIN_CONTROL_AGC 0x3FFD
#define AS5048A_CMD_MAGNITUDE 0x3FFE

#define AS5048A_MASK_READ_PACKAGE_PAR 0x8000
#define AS5048A_MASK_READ_PACKAGE_EF 0x4000
#define AS5048A_MASK_READ_PACKAGE_DATA 0x3FFF

#define AS5048A_FLAG_READ_PACKAGE_READ_FLAG 0x4000

spi_inst_t *as5048a_spi_port;
uint as5048a_spi_cs_pin;

void init_as5048a(spi_inst_t *spi_port, uint spi_cs_pin) {
    as5048a_spi_port = spi_port;
    as5048a_spi_cs_pin = spi_cs_pin;
}

uint16_t as5048a_calc_even_parity_flag(uint16_t package) {
    uint16_t count_1 = 0;
    for (uint8_t i = 0; i < 16; i++) {
        if (package>>i & 0x1) {
            count_1++;
        }
    }
    return count_1 & 0x1;
}

// returns 0 if parity is not valid
uint8_t as5048a_check_even_parity_flag(uint16_t package) {
    uint16_t actual_parity = (package & AS5048A_MASK_READ_PACKAGE_PAR) >> 15;
    uint16_t expected_parity = as5048a_calc_even_parity_flag(package & 0x7FFF);

    if (actual_parity != expected_parity) {
        return 0;
    }
    return 1;
}

uint8_t as5048a_check_error_flag(uint16_t package) {
    return package & AS5048A_MASK_READ_PACKAGE_EF >> 15;
}

uint16_t as5048a_extract_data(uint16_t package) {
    return package & AS5048A_MASK_READ_PACKAGE_DATA;
}

void as5048a_write_command_read_package(uint16_t addr) {
    uint16_t command_package = addr;
    command_package |= AS5048A_FLAG_READ_PACKAGE_READ_FLAG;
    command_package |= as5048a_calc_even_parity_flag(command_package) << 15;

    // write read commmand package
    spi_set_format(as5048a_spi_port, 16, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_put(as5048a_spi_cs_pin, 0);
    spi_write16_blocking(as5048a_spi_port, &command_package, 1);
    gpio_put(as5048a_spi_cs_pin, 1);
}

void as5048a_read_package(uint16_t *data) {
    // read package
    spi_set_format(as5048a_spi_port, 16, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_put(as5048a_spi_cs_pin, 0);
    spi_read16_blocking(as5048a_spi_port, AS5048A_NOP, data, 1);
    gpio_put(as5048a_spi_cs_pin, 1);
}

uint8_t as5048a_read(uint16_t addr, uint16_t *data) {
    as5048a_write_command_read_package(addr);

    as5048a_read_package(data);

    if (as5048a_check_even_parity_flag(*data) == 0) {
        return 0;
    }
    if (as5048a_check_error_flag(*data)) {
        return 0;
    }

    *data = as5048a_extract_data(*data);
    return 1;
}

uint8_t as5048a_read_angle(uint16_t *angle) {
    return as5048a_read(AS5048A_CMD_ANGLE, angle);
}

uint8_t as5048a_clear_error_flag(uint16_t *error_flag) {
    return as5048a_read(AS5048A_CMD_CLEAR_ERROR_FLAG, error_flag);
}

uint8_t as5048a_read_diagnostics_automatic_gain_control_agc(uint16_t *automatic_gain_control_agc) {
    return as5048a_read(AS5048A_CMD_DIAGNOSTICS_AUTOMATIC_GAIN_CONTROL_AGC, automatic_gain_control_agc);
}

uint8_t as5048a_read_magnitude(uint16_t *magnitude) {
    return as5048a_read(AS5048A_CMD_MAGNITUDE, magnitude);
}