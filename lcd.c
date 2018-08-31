#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

/*
  The NIBBLE_HIGH condition determines which PORT bits are used to
  transfer data to data bits 4-7 of the LCD.
  If LCD data bits 4-7 are connected to PORTD, bits 4-7, define NIBBLE_HIGH
  If LCD data bits 4-7 are connected to PORTD, bits 2-3 and PORTB bits 0-1,
  don't define NIBBLE_HIGH.
*/
#define NIBBLE_HIGH                 // Use bits 4-7 for talking to LCD

void lcd_init(void);
void lcd_moveto(unsigned char, unsigned char);
void lcd_stringout(char *);
void lcd_writecommand(unsigned char);
void lcd_writedata(unsigned char);
void lcd_writebyte(unsigned char);
void lcd_writenibble(unsigned char);
void lcd_wait(void);
void lcd_stringout_P(char *);

/*
  Use the "PROGMEM" attribute to store the strings in the ROM
  instead of in RAM.
*/
#ifdef NIBBLE_HIGH
const unsigned char str1[] PROGMEM = ">> at328-5.c hi <<901234";
#else
const unsigned char str1[] PROGMEM = ">> at328-5.c lo <<901234";
#endif
const unsigned char str2[] PROGMEM = ">> USC EE459L <<78901234";

#define LCD_RS          (1 << PB4)
#define LCD_RW          (1 << PB3)
#define LCD_E           (1 << PB2)
#define LCD_Bits        (LCD_RS|LCD_RW|LCD_E)

#ifdef NIBBLE_HIGH
#define LCD_Data_D     0xf0     // Bits in Port D for LCD data
#define LCD_Status     0x80     // Bit in Port D for LCD busy status
#else
#define LCD_Data_B     0x1c     // Bits in Port B for LCD data !!!need to change 00111000
#define LCD_Data_D     0x0c     // Bits in Port D for LCD data
#define LCD_Status     (1 << PD7) // Bit in Port D for LCD busy status
#endif


/*
  lcd_stringout_P - Print the contents of the character string "s" starting at LCD
  RAM location "x" where the string is stored in ROM.  The string must be
  terminated by a zero byte.
*/
void lcd_stringout_P(char *s)
{
    unsigned char ch;

    /* Use the "pgm_read_byte()" routine to read the date from ROM */
    while ((ch = pgm_read_byte(s++)) != '\0') {
        lcd_writedata(ch);             // Output the next character
    }
}

/*
  lcd_init - Do various things to force a initialization of the LCD
  display by instructions, and then set up the display parameters and
  turn the display on.
*/
void lcd_init(void)
{
#ifdef NIBBLE_HIGH
    DDRD |= LCD_Data_D;         // Set PORTD bits 4-7 for output
#else
    DDRB |= LCD_Data_B;         // Set PORTB bits 0-1 for output
    DDRD |= LCD_Data_D;         // Set PORTD bits 2-3 for output
#endif
    DDRB |= LCD_Bits;           // Set PORTB bits 2, 3 and 4 for output

    PORTB &= ~LCD_RS;           // Clear RS for command write

    _delay_ms(15);              // Delay at least 15ms

    lcd_writenibble(0x30);      // Send 00110000, set for 8-bit interface
    _delay_ms(5);               // Delay at least 4msec

    lcd_writenibble(0x30);      // Send 00110000, set for 8-bit interface
    _delay_us(120);             // Delay at least 100usec

    lcd_writenibble(0x30);      // Send 00110000, set for 8-bit interface
    _delay_ms(2);
    
    lcd_writenibble(0x20);      // Send 00100000, set for 4-bit interface
    _delay_ms(2);

    lcd_writecommand(0x28);     // Function Set: 4-bit interface, 2 lines

    lcd_writecommand(0x0f);     // Display and cursor on
}

/*
  lcd_moveto - Move the cursor to the row and column given by the arguments.
  Row is 0 or 1, column is 0 - 15.
*/
void lcd_moveto(unsigned char row, unsigned char col)
{
    unsigned char pos;
    pos = row * 0x40 + col;
    lcd_writecommand(0x80 | pos);
}

/*
  lcd_stringout - Print the contents of the character string "str"
  at the current cursor position.
*/
void lcd_stringout(char *str)
{
    char ch;
    while ((ch = *str++) != '\0')
	lcd_writedata(ch);
}

/*
  lcd_writecommand - Output a byte to the LCD command register.
*/
void lcd_writecommand(unsigned char x)
{
    PORTB &= ~LCD_RS;       // Clear RS for command write
    lcd_writebyte(x);
    lcd_wait();
}

/*
  lcd_writedata - Output a byte to the LCD data register
*/
void lcd_writedata(unsigned char x)
{
    PORTB |= LCD_RS;	// Set RS for data write
    lcd_writebyte(x);
    lcd_wait();
}

/*
  lcd_writebyte - Output a byte to the LCD
*/
void lcd_writebyte(unsigned char x)
{
    lcd_writenibble(x);
    lcd_writenibble(x << 4);
}

/*
  lcd_writenibble - Output a 4-bit nibble to the LCD
*/
void lcd_writenibble(unsigned char x)
{
#ifdef NIBBLE_HIGH
    PORTD &= ~LCD_Data_D;
    PORTD |= (x & LCD_Data_D);  // Put high 4 bits of data in PORTD
#else
    PORTB &= ~LCD_Data_B;
    PORTB |= (x & LCD_Data_B);  // Put low 2 bits of data in PORTB
    PORTD &= ~LCD_Data_D;
    PORTD |= (x & LCD_Data_D);  // Put high 2 bits of data in PORTD
#endif
    PORTB &= ~(LCD_RW|LCD_E);   // Set R/W=0, E=0
    PORTB |= LCD_E;             // Set E to 1
    PORTB |= LCD_E;             // Extend E pulse > 230ns
    PORTB &= ~LCD_E;            // Set E to 0
}

/*
  lcd_wait - Wait for the BUSY flag to reset
*/
void lcd_wait()
{
#ifdef USE_BUSY_FLAG
    unsigned char bf;

#ifdef NIBBLE_HIGH
    PORTD &= ~LCD_Data_D;       // Set for no pull ups
    DDRD &= ~LCD_Data_D;        // Set for input
#else
    PORTB &= ~LCD_Data_B;       // Set for no pull ups
    PORTD &= ~LCD_Data_D;
    DDRB &= ~LCD_Data_B;        // Set for input
    DDRD &= ~LCD_Data_D;
#endif

    PORTB &= ~(LCD_E|LCD_RS);   // Set E=0, R/W=1, RS=0
    PORTB |= LCD_RW;

    do {
        PORTB |= LCD_E;         // Set E=1
        _delay_us(1);           // Wait for signal to appear
        bf = PIND & LCD_Status; // Read status register high bits
        PORTB &= ~LCD_E;        // Set E=0
	PORTB |= LCD_E;         // Need to clock E a second time to fake
	PORTB &= ~LCD_E;        //   getting the status register low bits
    } while (bf != 0);          // If Busy (PORTD, bit 7 = 1), loop

#ifdef NIBBLE_HIGH
    DDRD |= LCD_Data_D;         // Set PORTD bits for output
#else
    DDRB |= LCD_Data_B;         // Set PORTB, PORTD bits for output
    DDRD |= LCD_Data_D;
#endif
#else
    _delay_ms(2);		// Delay for 2ms
#endif
}