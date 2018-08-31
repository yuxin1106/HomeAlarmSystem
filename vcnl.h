#include <avr/io.h>
#include <util/delay.h>

void i2c_init(uint8_t);
uint8_t i2c_io(uint8_t, uint8_t *, uint16_t,
               uint8_t *, uint16_t, uint8_t *, uint16_t);
void start_vcnl();
unsigned short int read_vcnl();