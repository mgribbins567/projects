#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include <string.h>
#include  "ports.h"

extern unsigned int switch1_press;
extern unsigned int switch2_press;
extern unsigned char sw_event;
extern volatile unsigned int debounce_count_sw1;
extern volatile unsigned int debounce_count_sw2;
extern unsigned char sw_press;
extern unsigned char sw_start;
extern unsigned char switch_count;

#pragma vector = PORT4_VECTOR

__interrupt void switchP4_interrupt(void) {
  if ((P4IFG & SW1) && (sw_event == READY)) {           // Switch 1 pressed
    P4IE &= ~SW1;
    P4IFG &= ~SW1;
    //TB0CTL &= ~TBIFG;
    TB0CCTL0 &= ~CCIE;
    TB0CCTL0 &= ~CCIFG;
    switch1_press++;                                    // Increment switch1_press
    sw_event = OFF;                                     // Set sw_event to OFF
    debounce_count_sw1 = 0;                             // Reset debounce counter
    //P6OUT &= ~LCD_BACKLITE;
    sw_press = SWITCH1;                                 // sw_press set to Switch 1
    switch_count = START;                               // switch_count set to start
    
    //display_sw();
    
    //sw_start = START;
  }
}

#pragma vector = PORT2_VECTOR

__interrupt void switchP2_interrupt(void) {
  if ((P2IFG & SW2) && (sw_event == READY)) {           // Switch 2 pressed
    P2IE &= ~SW2;
    P2IFG &= ~SW2;
    TB0CCTL0 &= ~CCIE;
    TB0CCTL0 &= ~CCIFG;
    switch2_press++;                                    // Increment switch1_press
    sw_event = OFF;                                     // Set sw_event to OFF
    debounce_count_sw2 = 0;                             // Reset debounce counter
    //P6OUT &= ~LCD_BACKLITE;
    sw_press = SWITCH2;                                 // sw_press set to Switch 2
    
    //display_sw();
    
    sw_start = START;                                   // sw_start set to start
  }
}