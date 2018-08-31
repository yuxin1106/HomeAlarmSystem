#include <avr/io.h>
#include <util/delay.h>

void lcd_init(void);
void lcd_moveto(unsigned char, unsigned char);
void lcd_stringout(char *);
void lcd_writecommand(unsigned char);
void lcd_writedata(unsigned char);
void lcd_writebyte(unsigned char);
void lcd_writenibble(unsigned char);
void lcd_wait(void);
void lcd_stringout_P(char *);
