void i2c_init(uint8_t);
uint8_t i2c_io(uint8_t, uint8_t *, uint16_t, 
               uint8_t *, uint16_t, uint8_t *, uint16_t);
void init_timer1(void);
void user_set_time(void);
void add_time(void);
void set_time_feature(void);
char compare_time(void);
void lcd_twodigits(uint8_t);
int bcd_to_decimal(uint8_t);
void real_time_init(void);

