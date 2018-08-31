#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <string.h>
#include "vcnl.h"
// #include "serial.h"
#include "buzzer_delay.h"

char get_light_sensor_output(void);
char get_ir_output(void);
char get_pir_output(void);
void serial_init();//unsigned short
void serial_stringout(char *, unsigned char);
void serial_txchar(char);
char rx_char();

#define FOSC 7372800            // Clock frequency = Oscillator freq.
#define BAUD 9600               // UART0 baud rate
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
#define BDIV (FOSC / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz
#define serial_str_size 16      // size of string to send 

volatile char msg_recv;
volatile char received_str[serial_str_size];
volatile unsigned char received_index;// = 0;
volatile char received_flag;// = 0;
volatile char received_char;
volatile char system_flag;//0 for system off
volatile char buzzer_flag;
volatile char intruder_flag;
volatile char window_flag;//1 for window open

int main(void){

	sei();
    serial_init();
    i2c_init(BDIV); 
	char ir_output;
	char light_output;
	char pir_output;
	
	system_flag = 1;

	DDRD |= (1 << PD3); // red LED output
	DDRD |= (1 << PD4); // yellow LED output (for room lights representation)
	DDRD |= (1 << PD7); // buzzer output
	DDRD |= (1 << PD6); // green system LED
	
	unsigned short int result_vcnl;
	PORTD &= ~(1 << PD6);
	while(1){
		if(system_flag){
			PORTD |= (1 << PD6);
			ir_output = get_ir_output();
			light_output = get_light_sensor_output();
			pir_output = get_pir_output();
			_delay_ms(2000);
			start_vcnl();
			result_vcnl = read_vcnl();
			_delay_ms(1000);
			start_vcnl();
			result_vcnl = read_vcnl();

			if(result_vcnl < 3000){//window is open
				window_flag = 1;
				serial_txchar('w');
			}
			else{
				//PORTD &= ~(1 << PD3);
				window_flag = 0;
			}

			//if sensors detect intruder, turn on red led and send signal
			if(ir_output & pir_output){
				PORTD |= (1 << PD3);//turn on the led
				intruder_flag = 1;
				serial_txchar('m');
			}
			else{
				intruder_flag = 0;
				//PORTD &= ~(1 << PD3);
			}
			
			if(window_flag | intruder_flag){//RED LED
				PORTD |= (1 << PD3);
			}

			// //if night time and there's an intruder, turn on lights. 
			if((!light_output) & (intruder_flag | window_flag) ){
				PORTD |= (1 << PD4);
			}
			else if(light_output & (intruder_flag | window_flag)){// day time
				buzzer_flag = 1;
				
			}
			// else{//else, turn off lights
			// 	PORTD &= ~(1 << PD4);
			// }

			while(buzzer_flag){
				play_note(2000);
				//_delay_ms(1000);
			}
		}
		else{
			PORTD &= ~(1 << PD6);
		}

		// if(received_flag){
  //           if(received_str[0]=='a'){
  //               if(received_str[1]=='b'){
  //                   PORTD |= (1 << PD3);
  //               }
  //               else{ 
  //               	PORTD |= (1 << PD4);
  //           	}
            
  //           }
  //           else{
  //               PORTD |= (1 << PD4);
  //           }
  //       }
	}
	

}

char get_ir_output(){

    if (PINB & (1 << PB0))
    {
       return 1;//something in between, pass the threshold  
    }
    else
    {
       return 0;
    }
    
}

char get_light_sensor_output(){

     if (PINB & (1 << PB1))
     {
        return 1;// detects light, represents day 
     }
     else
     {
        return 0;// night
     }  
 
}

char get_pir_output(){

  if ((PIND & (1 << PD2))){

    return 1;// detects object movement

  }
  else{

    return 0;

  }

}

//unsigned short ubrr_value
void serial_init()
{

    // Set up USART0 registers
     UBRR0 = MYUBRR;                       // Set baud rate
     UCSR0B |= (1 << TXEN0)|(1 << RXEN0);  // Enable RX and TX
     UCSR0C = (3 << UCSZ00);               // Async, no parity, 1 stop bit, 8 data bits
     UCSR0B |= (1 << RXCIE0);              // Enable receive interrupts
    // initialize global variables
    received_index = 0;
    received_flag = 0; 
}

void serial_txchar(char ch)
{
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = ch;
}

void serial_stringout(char *s, unsigned char size)
{
    // Call serial_txchar in loop to send a string
    int i = 0;
    while (i < size)
    {
       serial_txchar(s[i]);
       //lcd_moveto(0,8+i);
       //lcd_writedata(s[i]);
       _delay_ms(100);
       i++;
    }
    
}

char rx_char()
{
// Wait for receive complete flag to go high
    while ( !(UCSR0A & (1 << RXC0)) ) {}
    return UDR0;
}

ISR(USART_RX_vect)
{
    received_char = UDR0;
    if(received_char=='o'){//turn on the system
    	system_flag = 1;
    	PORTD |= (1 << PD6);
    }
    else if(received_char=='f'){//turn off the system
    	system_flag = 0; 
    }
    else if(received_char=='a'){//turn off the light or buzzer
    	window_flag = 0;
    	intruder_flag = 0;
    	buzzer_flag = 0;
    	PORTD &= ~(1 << PD4);
    	PORTD &= ~(1 << PD3);
    }
    
}


