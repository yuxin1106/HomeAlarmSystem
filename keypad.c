#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

// PC2-4 as Col1-3
// PC5, PB7, PC0-1 as Row1-4
// change PC4. PC5 to be PD2, PD3 because PC4&5 should be reserved for I2C
// PC2,PC3, PD2 as Col1-3
// PD3, PB7, PC0-1 as Row1-4

char get_key(void);
char *get_password(void);
// get password of 6 digits
char *get_password()
{
   lcd_moveto(0,0);
   lcd_stringout("Password:");
   
   char *password = (char *) malloc(sizeof(char) *7);
   char key;
   int i = 0;
   while (i < 6)
   {
     key = get_key();
     lcd_moveto(0, 9 + i);
     lcd_writedata(key);
     password[i] = key;
     i++;
   }
   password[6] = '\0';
   
   return password;
}

// get single key
char get_key()
{
   char key = 'N';
   while (key == 'N')
   {
      // set row 1 low
      DDRD |= (1 << PD3); // row1 output
      PORTD &= ~(1 << PD3); // set row 1 low
      _delay_ms(60);
      PORTC |= (1 << PC2)|(1 << PC3); // enable pull-up resistors
      PORTD |= (1 << PD2);
       _delay_ms(60);
      while (!(PINC & (1 << PC2))) // check if col1 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('1');
         key = '1';
      }
      while (!(PINC & (1 << PC3))) // check if col2 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('2');
         key = '2';
      }
      while (!(PIND & (1 << PD2))) // check if col3 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('3');
         key = '3';
      }
      //PORTC &= ~((1 << PC2)|(1 << PC3)|(1 << PC4)); // disable pull-up resistors
      
      // set row1 high, set row2 low
      PORTD |= (1 << PD3); // set row1 high
      DDRD &= ~(1 << PD3); // disable row1 output
      DDRB |= (1 << PB7);  // row2 output
      PORTB &= ~(1 << PB7); // set row2 low
      _delay_ms(60);
      PORTC |= (1 << PC2)|(1 << PC3); // enable pull-up resistors
      PORTD |= (1 << PD2);
      while (!(PINC & (1 << PC2))) // check if col1 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('4');
         key = '4';
      }
      while (!(PINC & (1 << PC3))) // check if col2 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('5');
         key = '5';
      }
      while (!(PIND & (1 << PD2))) // check if col3 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('6');
         key = '6';
      }
      //PORTC &= ~((1 << PC2)|(1 << PC3)|(1 << PC4)); // disable pull-up resistors
      
      // set row2 high, set row3 low
      PORTB |= (1 << PB7); // set row2 high
      DDRB &= ~(1 << PB7); // disable row2 output
      DDRC |= (1 << PC0);  // row3 output
      PORTC &= ~(1 << PC0); // set row3 low
      _delay_ms(60);
      PORTC |= (1 << PC2)|(1 << PC3); // enable pull-up resistors
      PORTD |= (1 << PD2);
      while (!(PINC & (1 << PC2))) // check if col1 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('7');
         key = '7';
      }
      while (!(PINC & (1 << PC3))) // check if col2 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('8');
         key = '8';
      }
      while (!(PIND & (1 << PD2))) // check if col3 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('9');
         key = '9';
      }
      //PORTC &= ~((1 << PC2)|(1 << PC3)|(1 << PC4)); // disable pull-up resistors
      
      // set row3 high, set row4 low
      PORTC |= (1 << PC0); // set row3 high
      DDRC &= ~(1 << PC0); // disable row3 output
      DDRC |= (1 << PC1);  // row4 output
      PORTC &= ~(1 << PC1); // set row4 low
      _delay_ms(60);
      PORTC |= (1 << PC2)|(1 << PC3); // enable pull-up resistors
      PORTD |= (1 << PD2);
      while (!(PINC & (1 << PC2))) // check if col1 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('*');
         key = '*';
      }
      while (!(PINC & (1 << PC3))) // check if col2 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('0');
         key = '0';
      }
      while (!(PIND & (1 << PD2))) // check if col3 is low
      {
         //lcd_moveto(0, 0);
         //lcd_writedata('#');
         key = '#';
      }
     // PORTC &= ~((1 << PC2)|(1 << PC3)|(1 << PC4)); // disable pull-up resistors
      
      // set row4 high
      PORTC |= (1 << PC1); // set row4 high
      DDRC &= ~(1 << PC1); // disable row4 output
      _delay_ms(60);
   }
    return key;
}