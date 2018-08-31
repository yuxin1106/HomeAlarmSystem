#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "lcd.h"
#include "keypad.h"
#include "set_time.h"

//volatile uint8_t data[6];// = {0x00, 0x51, 0x08, 0x13, 0x31, 0x08};
volatile uint8_t adata[1];
volatile uint8_t rdata[3];
volatile uint8_t status;
volatile int iter;// = 0;
volatile char wrong_time_flag;// = 0;
volatile char user_time[5];
volatile int rtc_user_time[3];
volatile int added_time[3];
volatile int real_time_display_flag; 
volatile int set_time_display_flag;

//global variable defined for this file
volatile char button_pressed_flag = 0;
volatile char system_on_flag = 1; 
volatile char *password_set;
volatile char *password_entered;
volatile char time_met_flag = 0;
volatile int password_correct_flag;

void splash_screen(void);
void button_init(void);
void set_password(void);
void reset_password(void);
void check_password(void);

int main(void)
{
    sei();
    lcd_init();
    //splash_screen();
    // system first turned on, request password from user
    //set_password();
    
    button_init();
 
    real_time_init();
    
    char single_key;
    
    
    while (1)
    {
        
        single_key = get_key();
        // if user want to reset password press '#'
        if (single_key == '#')
        {
           real_time_display_flag = 0;
           reset_password();
        }
        else if (single_key == '*')
        {
           set_time_feature();
        }
        _delay_ms(100);
        // if the time set by the user is met, clear the screen
        lcd_moveto(1,0);
        lcd_writedata(time_met_flag + '0');
        
        if (time_met_flag)
        {
          //lcd_stringout("System");
           //lcd_writecommand(1);
           if (system_on_flag)
           {
              system_on_flag = 0;
              lcd_moveto(1,0);
              lcd_stringout("System Off");
           }
           else
           {
              system_on_flag = 1;
              lcd_moveto(1,0);
              lcd_stringout("System On");
           }
           
           time_met_flag = 0;
        }
        //lcd_stringout("System");
        
        // ask user to enter password until user gets correct
        //while (!check_password()){};
        //password_correct_flag = check_password();
        //check_password();
        
    }
    
    return 0;

}
// display splash screen for the system
void splash_screen()
{
    lcd_moveto(0,0);
    lcd_stringout("  Home  Alarm");
    lcd_moveto(1,0);
    lcd_stringout("    System    ");
    _delay_ms(1000);
    lcd_writecommand(1);
}
// request password from user the first time
void set_password()
{
    password_set = get_password();
    lcd_writecommand(1);
    lcd_moveto(1,0);
    lcd_stringout("Password Set");
    _delay_ms(1000);
    lcd_writecommand(1);
}
// reset password
void reset_password()
{
    password_set = get_password();
    lcd_writecommand(1);
    lcd_moveto(1,0);
    lcd_stringout("Password Reset");
    _delay_ms(1000);
    lcd_writecommand(1);
}
// check password entered by the user if correct or not
void check_password()
{
    real_time_display_flag = 0;
    password_entered = get_password();
    lcd_moveto(1,0);
    lcd_stringout(password_entered);
    //lcd_writecommand(1);
    
    if (password_entered == password_set) // ??
    {
       lcd_moveto(0,0);
       lcd_stringout("Password Right");
       //return 1;
       password_correct_flag = 1;
    }
    else
    {
       lcd_moveto(0,0);
       lcd_stringout("Password Wrong");
       //return 0;
       password_correct_flag = 0;
    }
    
    // ask user to enter password until user gets correct
}
// initialize pin interrupt
void button_init()
{
   //initialize PB1 as input - pull up resistor
   PORTB |= (1 << PB1);
   //enable pin interrupt for PB1
   PCICR |= (1 << PCIE0);
   PCMSK0 |= (1 << PCINT1);

}
// pin interrupt for button press PORTB 
ISR(PCINT0_vect)
{
    if (!(PINB & (1 << PB1)))
    {
      button_pressed_flag = 1;
      //lcd_moveto(0,0);
      //lcd_stringout("Button Pressed");
    }
    /*else
    {
      lcd_moveto(0,0);
      lcd_stringout("Button Not Pressed");
    }*/

}



