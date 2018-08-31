#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "lcd.h"
#include "keypad.h"
#include "set_time.h"

#define FOSC 7372800            // Clock frequency = Oscillator freq.
#define BAUD 9600               // UART0 baud rate
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
#define BDIV (FOSC / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz
#define DS1307_ADDR 0xD0 // I2C bus address of DS1307 RTC

//extern volatile uint8_t data[6];// = {0x00, 0x51, 0x08, 0x13, 0x31, 0x08};
extern volatile uint8_t adata[1];
extern volatile uint8_t rdata[3];
extern volatile uint8_t status;
extern volatile int iter;// = 0;
extern volatile char wrong_time_flag;// = 0;
extern volatile char user_time[5];
extern volatile int rtc_user_time[3];
extern volatile int added_time[3];
extern volatile int real_time_display_flag; 
extern volatile int set_time_display_flag;
extern volatile char system_on_flag;
extern volatile char time_met_flag;

void real_time_init()
{
   uint8_t data[6] = {0x00, 0x51, 0x08, 0x13, 0x31, 0x08};
   iter = 0;
   wrong_time_flag = 0;
   real_time_display_flag = 1;

   lcd_init();
   i2c_init(BDIV);
   sei();
   init_timer1();
   adata[0] = 0x00;
    // write current time
   status = i2c_io(DS1307_ADDR, adata, 1, data, 6, NULL, 0);
}

void lcd_twodigits(uint8_t data)
// helper function for WriteDate()
// input is two digits in BCD format
// output is to LCD display at current cursor position
{
     uint8_t temp = data>>4;
     lcd_writedata(temp+'0');
     data &= 0x0F;
     lcd_writedata(data+'0');
}

// bcd format from rtc to decimal
int bcd_to_decimal(uint8_t x)
{
    return x - 6 * (x >> 4);
}

// compare time
char compare_time()
{
    return (added_time[2] == bcd_to_decimal(rdata[2])) && (added_time[1] == bcd_to_decimal(rdata[1])) && (added_time[0] == bcd_to_decimal(rdata[0]));
}

// user_set_time, save current time, add_time routine
void set_time_feature()
{
     set_time_display_flag = 1;
     user_set_time();
     set_time_display_flag = 0;
     rtc_user_time[2]  = bcd_to_decimal(rdata[2]);
     rtc_user_time[1]  = bcd_to_decimal(rdata[1]);
     rtc_user_time[0]  = bcd_to_decimal(rdata[0]);
     add_time();
     while(1){
        if(compare_time()){
          time_met_flag = 1; 
          break;
        }
     }
     
}

void user_set_time()
{
    //char user_time[5];
    // reading data from rtc 
        lcd_moveto(1,0);
        lcd_stringout("Set Time:");
        lcd_moveto(1,9);
        
        char key;
        while (iter < 5)
        {
           //lcd_moveto(1, 9+iter);
           // let user input time in terms of hours#minutes
           key = get_key();
           lcd_writedata(key);
           
           // user can reset time by pressing '*'
           if (key == '*')
           {
              wrong_time_flag = 1;
           }
           
           if ((iter == 0)||(iter == 1))
           {
              if (((key > '2') && (iter == 0))||((user_time[0] == '2') && (key > '4')))
              {
                 wrong_time_flag = 1;
                 //iter = 0;
              }
              else 
              {
                //user_hour[iter] = key;
                user_time[iter] = key;
                iter++;
              }
           }
           else if ((iter == 3)||(iter == 4))
           {
              if ((key > '5') && (iter == 3))
              {
                wrong_time_flag = 1;
                //iter = 0;
              }
              else
              {
                //user_minute[iter-3] = key;
                user_time[iter] = key;
                iter++;
              }
           }
           else if (iter == 2)
           {
              user_time[iter] = '#';
              iter++;
           }
           //password[iter] = key;
           
        }
        lcd_moveto(1,14);
        lcd_writedata(' ');
        lcd_moveto(1,15);
        lcd_writedata(' ');

    lcd_writecommand(1);
    lcd_moveto(1,0);
    //lcd_stringout("               ");
    //lcd_stringout(user_time);
    //return user_time;
}

/* add user input and the current time */
void add_time()
{
    int user_hour, user_minute;
    sscanf(user_time, "%d#%d", &user_hour, &user_minute);
    
    //uint8_t user_hour_t = user_hour;
   // uint8_t user_minute_t = user_minute;
    
    int rtc_user_time_hour = rtc_user_time[2];
    int rtc_user_time_minute = rtc_user_time[1];
    int rtc_user_time_second = rtc_user_time[0];
    
    added_time[2] = user_hour + rtc_user_time_hour;
    added_time[1] = user_minute + rtc_user_time_minute;
    added_time[0] = rtc_user_time_second;
    
    if (added_time[1] > 59)
    {
       added_time[1] = added_time[1] - 60;
       added_time[2]++;
    }
     
    if (added_time[2] > 23)
    {
       added_time[2] = added_time[2] - 24;
    }
    /*
    char buffer[17];
    snprintf(buffer, 17, "%d:%d:%d", added_time[2], added_time[1], added_time[0]);
    lcd_stringout(buffer);
    */
}

/* initialize timer*/
void init_timer1(){
     TCCR1B|=(1 << WGM12); // set to CTC mode
     TIMSK1|=(1 << OCIE1A); // enable timer interrupt, local interrupt bit=1
     OCR1A = 11520;
     TCCR1B|=(1 << CS11)|(1 << CS10); // 0.1s, pre-scaler=64, OCR1A=25000, 16mhz/64*0.1
}

ISR(TIMER1_COMPA_vect){
    
    status = i2c_io(DS1307_ADDR, adata, 1, NULL, 0, rdata, 3);
    
    if (real_time_display_flag)
    {
       lcd_moveto(0,0);
       lcd_twodigits(rdata[2]);
       lcd_writedata(':');
       lcd_twodigits(rdata[1]);
       lcd_writedata(':');
       lcd_twodigits(rdata[0]);     
    }
    
    
    if (wrong_time_flag)
    {
       iter = 0; 
       lcd_moveto(1,9);
       lcd_stringout("      ");
       wrong_time_flag = 0;
    }

    if (set_time_display_flag) 
    {
       lcd_moveto(1,9+iter);
    }

    // compare time
    //lcd_moveto(1,0);
    //lcd_writedata(compare_time());
    /*
    if (compare_time() && !system_on_flag)
    {
        lcd_moveto(1,0);
        lcd_stringout("System on");
        time_met_flag = 1;
        system_on_flag = 1;
        //lcd_writecommand(1);
    }
    else if (compare_time() && system_on_flag)
    {
        lcd_moveto(1,0);
        lcd_stringout("System off");
        time_met_flag = 1;
        system_on_flag = 0;
        //lcd_writecommand(1);
    }       
    */
    // if (compare_time())
    // {
    //    time_met_flag = 1;    
    //    //lcd_moveto(1,0);
    //    //lcd_writedata(time_met_flag + '0');
    //    // lcd_moveto(1,3);
    //    // lcd_writedata()
    // }
    
    // if (time_met_flag)
    // {
        
    //     if (system_on_flag)
    //     {
    //        lcd_moveto(1,0);
    //        lcd_stringout("System Off");
    //        system_on_flag = 1;
    //     }
    //     else
    //     {
    //        lcd_moveto(1,0);
    //        lcd_stringout("System On");
    //        system_on_flag = 0;
           
    //     }
    
    // }
}


uint8_t i2c_io(uint8_t device_addr, uint8_t *ap, uint16_t an, 
               uint8_t *wp, uint16_t wn, uint8_t *rp, uint16_t rn)
{
    uint8_t status, send_stop, wrote, start_stat;

    status = 0;
    wrote = 0;
    send_stop = 0;

    if (an > 0 || wn > 0) {
        wrote = 1;
        send_stop = 1;

        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);  // Send start condition
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != 0x08)                 // Check that START was sent OK
            return(status);

        TWDR = device_addr & 0xfe;          // Load device address and R/W = 0;
        TWCR = (1 << TWINT) | (1 << TWEN);  // Start transmission
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != 0x18) {               // Check that SLA+W was sent OK
            if (status == 0x20)             // Check for NAK
                goto nakstop;               // Send STOP condition
            return(status);                 // Otherwise just return the status
        }

        // Write "an" data bytes to the slave device
        while (an-- > 0) {
            TWDR = *ap++;                   // Put next data byte in TWDR
            TWCR = (1 << TWINT) | (1 << TWEN); // Start transmission
            while (!(TWCR & (1 << TWINT))); // Wait for TWINT to be set
            status = TWSR & 0xf8;
            if (status != 0x28) {           // Check that data was sent OK
                if (status == 0x30)         // Check for NAK
                    goto nakstop;           // Send STOP condition
                return(status);             // Otherwise just return the status
            }
        }

        // Write "wn" data bytes to the slave device
        while (wn-- > 0) {
            TWDR = *wp++;                   // Put next data byte in TWDR
            TWCR = (1 << TWINT) | (1 << TWEN); // Start transmission
            while (!(TWCR & (1 << TWINT))); // Wait for TWINT to be set
            status = TWSR & 0xf8;
            if (status != 0x28) {           // Check that data was sent OK
                if (status == 0x30)         // Check for NAK
                    goto nakstop;           // Send STOP condition
                return(status);             // Otherwise just return the status
            }
        }

        status = 0;                         // Set status value to successful
    }

    if (rn > 0) {
        send_stop = 1;

        // Set the status value to check for depending on whether this is a
        // START or repeated START
        start_stat = (wrote) ? 0x10 : 0x08;

        // Put TWI into Master Receive mode by sending a START, which could
        // be a repeated START condition if we just finished writing.
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
                                            // Send start (or repeated start) condition
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != start_stat)           // Check that START or repeated START sent OK
            return(status);

        TWDR = device_addr  | 0x01;         // Load device address and R/W = 1;
        TWCR = (1 << TWINT) | (1 << TWEN);  // Send address+r
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != 0x40) {               // Check that SLA+R was sent OK
            if (status == 0x48)             // Check for NAK
                goto nakstop;
            return(status);
        }

        // Read all but the last of n bytes from the slave device in this loop
        rn--;
        while (rn-- > 0) {
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); // Read byte and send ACK
            while (!(TWCR & (1 << TWINT))); // Wait for TWINT to be set
            status = TWSR & 0xf8;
            if (status != 0x50)             // Check that data received OK
                return(status);
            *rp++ = TWDR;                   // Read the data
        }

        // Read the last byte
        TWCR = (1 << TWINT) | (1 << TWEN);  // Read last byte with NOT ACK sent
        while (!(TWCR & (1 << TWINT)));     // Wait for TWINT to be set
        status = TWSR & 0xf8;
        if (status != 0x58)                 // Check that data received OK
            return(status);
        *rp++ = TWDR;                       // Read the data

        status = 0;                         // Set status value to successful
    }
    
nakstop:                                    // Come here to send STOP after a NAK
    if (send_stop)
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);  // Send STOP condition

    return(status);
}


/*
  i2c_init - Initialize the I2C port
*/
void i2c_init(uint8_t bdiv)
{
    TWSR = 0;                           // Set prescalar for 1
    TWBR = bdiv;                        // Set bit rate register
}

































