#include "as5048a.h"

#define AS5048A_NOP 0x0000

#define AS5048A_CMD_ANGLE                                  0x3FFF
#define AS5048A_CMD_CLEAR_ERROR_FLAG                       0x0001
#define AS5048A_CMD_DIAGNOSTICS_AUTOMATIC_GAIN_CONTROL_AGC 0x3FFD
#define AS5048A_CMD_MAGNITUDE                              0x3FFE
#define AS5048A_CMD_OTP_REGISTER_ZERO_POSITION_HI          0x0016
#define AS5048A_CMD_OTP_REGISTER_ZERO_POSITION_LOW         0x0017

#define AS5048A_MASK_READ_PACKAGE_PAR  0x8000
#define AS5048A_MASK_READ_PACKAGE_EF   0x4000
#define AS5048A_MASK_READ_PACKAGE_DATA 0x3FFF

// hardware errors
#define AS5048A_MASK_FRAMING_ERROR   0x0001
#define AS5048A_MASK_COMMAND_INVALID 0x0002
#define AS5048A_MASK_PARITY_ERROR    0x0004

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

void as5048a_send_command_write_package(uint16_t addr) {
    uint16_t command_package = 0;
    command_package |= (addr & 0x3FFF);
    command_package |= as5048a_calc_even_parity_flag(command_package) << 15;

    gpio_put(as5048a_spi_cs_pin, 0);
    spi_write16_blocking(as5048a_spi_port, &command_package, 1);
    gpio_put(as5048a_spi_cs_pin, 1);
}

void as5048a_send_write_package(uint16_t data) {
    
}

void as5048a_send_command_read_package(uint16_t addr) {
    uint16_t command_package = addr;
    command_package |= AS5048A_FLAG_READ_PACKAGE_READ_FLAG;
    command_package |= as5048a_calc_even_parity_flag(command_package) << 15;

    gpio_put(as5048a_spi_cs_pin, 0);
    spi_write16_blocking(as5048a_spi_port, &command_package, 1);
    gpio_put(as5048a_spi_cs_pin, 1);
}

void as5048a_recv_read_package(uint16_t *data) {
    gpio_put(as5048a_spi_cs_pin, 0);
    spi_read16_blocking(as5048a_spi_port, AS5048A_NOP, data, 1);
    gpio_put(as5048a_spi_cs_pin, 1);
}

uint8_t as5048a_read(uint16_t addr, uint16_t *data) {
    as5048a_send_command_read_package(addr);

    as5048a_recv_read_package(data);

    if (as5048a_check_even_parity_flag(*data) == 0) {
        return AS5048A_ERROR_PARITY;
    }
    if (as5048a_check_error_flag(*data)) {
        return AS5048A_ERROR_FLAG;
    }

    *data = as5048a_extract_data(*data);
    return AS5048A_OK;
}

uint8_t as5048a_read_otp_register_zero_position(uint16_t *otp_zero_position) {
    uint16_t otp_zero_position_hi = 0;
    uint8_t error = as5048a_read(AS5048A_CMD_OTP_REGISTER_ZERO_POSITION_HI, &otp_zero_position_hi);
    if (error != AS5048A_OK) {
        return error;
    }

    uint16_t otp_zero_position_low = 0;
    error = as5048a_read(AS5048A_CMD_OTP_REGISTER_ZERO_POSITION_LOW, &otp_zero_position_low);
    if (error != AS5048A_OK) {
        return error;
    }

    *otp_zero_position = 0;
    *otp_zero_position = *otp_zero_position | (otp_zero_position_hi & 0x00FF) << 6;
    *otp_zero_position = *otp_zero_position | (otp_zero_position_low & 0x003F);
    return AS5048A_OK;
}

uint8_t as5048a_write_otp_register_zero_position(uint16_t otp_zero_position) {
    uint16_t otp_zero_position_high = 0;
    otp_zero_position_high = (otp_zero_position & 0x3FC0) >> 6;

    uint16_t otp_zero_position_low = 0;
    otp_zero_position_low = (otp_zero_position & 0x003F);


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

bool as5048a_is_parity_error(uint8_t error) {
    return error == AS5048A_ERROR_PARITY;
}

bool as5048a_is_hardware_error(uint8_t error) {
    return error == AS5048A_ERROR_FLAG;
}

bool as5048a_is_hardware_parity_error(uint16_t error) {
    return (error & AS5048A_MASK_PARITY_ERROR) >> 2 == 1;
}

bool as5048a_is_hardware_command_invalid_error(uint16_t error) {
    return (error & AS5048A_MASK_COMMAND_INVALID) >> 1 == 1;
}

bool as5048a_is_hardware_framing_error(uint16_t error) {
    return (error & AS5048A_MASK_FRAMING_ERROR) == 1;
}