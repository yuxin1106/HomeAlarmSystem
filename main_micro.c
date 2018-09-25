#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "lcd.h"
#include "keypad.h"
#include "set_time.h"
//#include "buzzer.h"
#include "serial.h"
#include "buzzer_interrupt.h"

#define DS1307_ADDR 0xD0
#define serial_str_size 1      // size of string to send 
#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register

//volatile uint8_t data[6];// = {0x00, 0x51, 0x08, 0x13, 0x31, 0x08};
//global variable used for real time
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
volatile char comp_one;

//global variable used for serial
volatile char received_str[serial_str_size];
volatile unsigned char index;// = 0;
volatile char received_flag;// = 0;


//global variable defined for this file
volatile char button_pressed_flag = 0;
volatile char system_on_flag = 1; 
volatile char *password_set;
volatile char *password_entered;
volatile char time_met_flag = 0;
volatile char password_correct_flag = 0;
volatile char buzzer_on_flag = 0;

void splash_screen(void);
void button_init(void);
void set_password(void);
void reset_password(void);
void check_password(void);
void keypad_interrupt_init(void);
//void buzzer_on(void);

int main(void)
{
   
    sei();
    lcd_init();
    splash_screen();
    lcd_moveto(0,0);
    serial_init(MYUBRR);
    set_password();
    
    _delay_ms(2000);
    serial_txchar('o');
    lcd_moveto(1,0);
    lcd_stringout("System On");
    //lcd_moveto(0,0);
    //lcd_writedata('o');
    //splash_screen();
    // system first turned on, request password from user
    //set_password();
    
    button_init();
 
    real_time_init();
    //set_time_feature();
    keypad_interrupt_init();
    char single_key;
    
    //buzzer_init();
    //buzzer_on_flag = 1;
    
    //init_timer0();
    
    while (1)
    {
        
        /*
        if (received_flag)
        {
            
        
        }
        */
        //single_key = get_key();
        //lcd_moveto(1,0);
        //lcd_writedata(buzzer_on_flag + '0');
        
        //buzzer_on();
        // buzzer_on_flag updated correctly, but not controlling the while loop properly
        /*
        while (buzzer_on_flag)
        {
           play_note(4000);
        }
        */
        /*
        while (1)
        {
           play_note(4000);
           lcd_moveto(1,0);
           lcd_writedata(buzzer_on_flag + '0');
           if (!buzzer_on_flag)
           {
              break;
           }
        }
        */
        /*
        if (time_met_flag)
        {
           system_on_flag ^= system_on_flag;
           //real_time_display_flag = 0;
           //lcd_writecommand(1);
           time_met_flag = 0;
        }
        
        if (system_on_flag)
        {
            lcd_moveto(1,0);
            lcd_stringout("System On");
        }
        else
        {
            lcd_moveto(1,0);
            lcd_stringout("System Off");
        }
        */
        
       // single_key = get_key();
        // if user want to reset password press '#'
       // if (single_key == '#')
       // {
       //    real_time_display_flag = 0;
       //    reset_password();
       // }
       // else if (single_key == '*')
       // {
       //    set_time_feature();
       // }
    
       
        // ask user to enter password until user gets correct
        //while (!check_password()){};
        //password_correct_flag = check_password();
        //check_password();
        single_key = get_key();
    }
    
   

}
/*
void buzzer_on()
{
    while (buzzer_on_flag)
    {
       play_note(4000);
    }
}
*/
// display splash screen for the system
void splash_screen()
{
    lcd_moveto(0,0);
    lcd_stringout("  Home  Alarm");
    lcd_moveto(1,0);
    lcd_stringout("    System    ");
    _delay_ms(3000);
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
    _delay_ms(1000);    // may cause problem for interrupts
    lcd_writecommand(1);
}
// check password entered by the user if correct or not
void check_password()
{
    //lcd_writecommand(1);
    real_time_display_flag = 0;
    password_entered = get_password();
    //lcd_moveto(1,0);
    //lcd_stringout(password_entered);
    //lcd_writecommand(1);
    
    //if (password_entered == password_set) // ??
    if (!strcmp(password_entered, password_set))
    {
       lcd_moveto(1,0);
       lcd_stringout("Password Right");
       lcd_moveto(0,0);
       lcd_stringout("                ");
       //return 1;
       password_correct_flag = 1;
    }
    else
    {
       lcd_moveto(1,0);
       lcd_stringout("Password Wrong");
       lcd_moveto(0,0);
       lcd_stringout("                ");
       //lcd_moveto(1,0);
       //lcd_stringout("Try again");
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
      //disable_timer0();
      
      // when button pressed, request password from user
      //check_password();
      /*
      while (1)
      {
          play_note(4000);
          while (!password_correct_flag)
          {
              play_note(4000);
              check_password();
          }
          if (password_correct_flag)
          {
              break;
          }
      
      }
      */
      
      while (!password_correct_flag)
      {
          check_password();
          //lcd_moveto(1,0);
          //lcd_writedata(buzzer_on_flag + '0');
      }
      
       
        
      if (password_correct_flag)
      {
         buzzer_on_flag = 0;
         real_time_display_flag = 1;
         //disable_timer0();
         //lcd_moveto(1,0);
         //lcd_writedata(buzzer_on_flag + '0');
         if (system_on_flag && !received_flag)
         {
            real_time_display_flag = 1;
            system_on_flag = 0;
            lcd_moveto(1,0);
            lcd_stringout("                ");
            lcd_moveto(1,0);
            lcd_stringout("System Off");
            serial_txchar('f');
         }
         else if (!system_on_flag && !received_flag)
         {
            real_time_display_flag = 1;
            system_on_flag = 1;
            lcd_moveto(1,0);
            lcd_stringout("                ");
            lcd_moveto(1,0);
            lcd_stringout("System On");
            serial_txchar('o');
         }
         
         // if user want to turn off buzzer/light
         if (received_flag)
         {
            serial_txchar('a');
            received_flag = 0;
            lcd_moveto(1,0);
            lcd_stringout("                ");
            if (system_on_flag)
            {
               real_time_display_flag = 1;
               lcd_moveto(1,0);
               lcd_stringout("System On");
            }
            else
            {
               real_time_display_flag = 1;
               lcd_moveto(1,0);
               lcd_stringout("System Off");
            }
         }
      }

      //lcd_moveto(1,0);
      //lcd_writedata(buzzer_on_flag + '0');
      // reset password_correct_flag for next button press
      password_correct_flag = 0;
      
      
    }


}
void keypad_interrupt_init()
{
   // enable pin interrupt for row4 and col1, PC1, PC2
   PCICR |= (1 << PCIE1);
   PCMSK1 |=(1 << PCINT10);//|(1 << PCINT9);
   //PCMSK1 |=(1 << PCINT11)|(1 << PCINT9);
}

// pin interrupt for keypad '*'
ISR(PCINT1_vect)
{
   //lcd_moveto(0,0);
   //lcd_stringout("Pin interrupt");
   /*
   // '0' pressed, row4, col2, PC1, PC3
   if (((PINC & (1 << PC1)) == 0) && ((PINC & (1 << PC3)) == 0))
   {
      lcd_moveto(1,0);
      lcd_stringout("0 pressed");
   
   }
   */
   
   
   // * pressed
   if ((!(PINC & (1 << PC1)))&& (!(PINC & (1 << PC2))))
   {
      //lcd_moveto(1,0);
      //lcd_stringout("* pressed");
      real_time_display_flag = 1;
      //TCCR1B &= ~((1 << CS11)|(1 << CS10));
      set_time_feature();
      //init_timer1();
      //lcd_moveto(1,0);
      //char buf[17];
      //snprintf(buf, 17, "%d:%d:%d", added_time[2], added_time[1], added_time[0]);
     // lcd_stringout(buf);
     
     
   }
   
   // # pressed
   
}

ISR(USART_RX_vect)
{
    //lcd_moveto(1,0);
    //lcd_stringout("RX interrupt");
    // Handle received character
    char received_char = UDR0;
    lcd_moveto(1,0);
    lcd_writedata(received_char);
    //lcd_moveto(0,8+index);
    //lcd_writedata(received_char);
    if (received_char == 'm')
    {
       lcd_writecommand(1);
       real_time_display_flag = 1;
       lcd_moveto(1,0);
       lcd_stringout("Moving Objects");
    }
    else if (received_char == 'w')
    {
       lcd_writecommand(1);
       real_time_display_flag = 1;
       lcd_moveto(1,0);
       lcd_stringout("Window Intrusion");    
    }
    else if (received_char == 'e')
    {
       lcd_writecommand(1);
       real_time_display_flag = 1;
       lcd_moveto(1,0);
       lcd_stringout("Sending Email"); 
    
    }
    
    
    //received_str[index] = received_char;
    received_flag = 1;
    /*
     if ((received_char == 'm')||(received_char == 'w'))
     {
       while (!password_correct_flag)
       {
          check_password();
       }
      
       if (password_correct_flag)
       {
         serial_txchar('a');
       }
       
       password_correct_flag = 0;
    }
      */
      
    //index++;
    /*
    if (index == 2)
    {
       received_flag = 1;
       index = 0;
    }
    */
}
