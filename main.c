/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ~~~~~~~~~~ Micro-Controller based Over Voltage Protection Circuit [Rev. 1.0] ~~~~~~~~~~
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 * Warning: Working with 220 V ac mains voltage is dangerous and might be lethal!
 * Please Do NOT Try by Yourself!
 * 
 * This code is under MIT License
 * Copyright (c) 2021 Sayantan Sinha
*/

#include <htc.h>

__CONFIG(OSC_IntRC & WDT_OFF & CP_OFF & MCLRE_OFF);

#define _XTAL_FREQ 4000000      // f_cpu = 4 MHz

#define V_SEN GP0               // Assign pin for voltage sense
#define LED_HV GP1              // Assign pin for high-voltage indicator LED
#define RLY_ON GP2              // Assign pin for relay-ON
#define OP_SEN GP3              // Assign pin for output sense
#define RLY_OFF GP4             // Assign pin for relay-OFF

#define TIMER0CON 0b11010000    // TOCS = 0, PSA = 0 (clk source = internal (f_cpu /4), assign the prescaler to Timer0)

unsigned char isVsenLow(void);
void relayOn(void);
void relayOff(void);
void blink(unsigned char nTimes);

void main(void)
{
  GPIO = 0;
  TRIS = 0b00001001;           // V_SEN, OP_SEN = Input; RLY_ON, RLY_OFF and LED_HV = Output
  
  unsigned char status = 0;    // status = 0 : Power off, status = 1 : Power on
  
  while(1) {
    if(status == 0) {
      LED_HV = 1;
      while(!isVsenLow());     // Wait until V_SEN is read low for 200 ms
      __delay_ms(3000);        // Wait 3 s
      if(isVsenLow()) {        // Check if V_SEN still reads low for 200 ms
        relayOn();             // Turn on relay
        status = 1;
      }
    }
    else {
      if(V_SEN) {
        __delay_ms(2);
        if(V_SEN) {
          relayOff();
          status = 0;
        }
      }
      if(!OP_SEN) {            // If for any reason output is Low
        __delay_ms(2);
        if(!OP_SEN)
          status = 0;
      }
    }
  }
  return;
}

unsigned char isVsenLow(void)
{
  unsigned char tick1ms = 0;
  TMR0 = 0;                    // Reset Timer0
  OPTION = TIMER0CON | 0x04;   // Prescaler = 32; T = (1/4 MHz) * 4 * 32 = 32 us
  
  while (tick1ms < 200) {
    if (V_SEN)                 // If V_SEN is high return with 0
      return(0);
    if(TMR0 >= 30) {
      TMR0 = 0;
      tick1ms++;
    }
  }
  return(1);                   // V_SEN was low for 200 ms, return 1
}

void relayOn(void)
{
  RLY_OFF = 1;                 // Trigger Turn-off relay. So that the Turn on relay does not have to take the output load
  __delay_ms(3);
  RLY_ON = 1;
  __delay_ms(3);
  RLY_OFF = 0;                 // Release Turn-off relay to engage the NVC from O/P
  __delay_ms(300);
  RLY_ON = 0;
  LED_HV = 0;
}

void relayOff(void)
{
  RLY_OFF = 1;
  __delay_ms(300);
  RLY_OFF = 0;
  LED_HV = 1;
  blink(5);                    // Blink the High voltage indicator LED 5 times
}

void blink(unsigned char nTimes)
{
  nTimes *= 2;
  while(nTimes) {
    __delay_ms(1000);
    LED_HV = ~LED_HV;
    nTimes--;
  }
}