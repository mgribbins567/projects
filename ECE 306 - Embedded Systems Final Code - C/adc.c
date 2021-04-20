#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include <string.h>
#include  "ports.h"

extern char adc_char[10];
extern char display_char[6];
extern unsigned int ADC_Channel;
extern unsigned int ADC_Left_Detect;
extern unsigned int ADC_Right_Detect;
extern unsigned int ADC_Thumb_Detect;
extern unsigned int fr_event;
extern unsigned int travel_count;
extern char display_line[4][11];
extern volatile unsigned char update_display;
extern unsigned char adc_line;
extern unsigned int switch1_press;
extern unsigned char sw_start;
extern unsigned int Right_Turn_Speed;
extern unsigned int Left_Turn_Speed;
extern unsigned int display_timer;

void Init_ADC(void) {
//------------------------------------------------------------------------------
// V_DETECT_L (0x04) // Pin 2 A2
// V_DETECT_R (0x08) // Pin 3 A3
// V_THUMB (0x20) // Pin 5 A5
//------------------------------------------------------------------------------


// ADCCTL0 Register
  ADCCTL0 = 0;          // Reset
  ADCCTL0 |= ADCSHT_2;  // 16 ADC clocks
  ADCCTL0 |= ADCMSC;    // MSC
  ADCCTL0 |= ADCON;     // ADC ON
  
// ADCCTL1 Register
  ADCCTL2 = 0;                  // Reset
  ADCCTL1 |= ADCSHS_0;          // 00b = ADCSC bit
  ADCCTL1 |= ADCSHP;            // ADC sample-and-hold SAMPCON signal from sampling timer.
  ADCCTL1 &= ~ADCISSH;          // ADC invert signal sample-and-hold.
  ADCCTL1 |= ADCDIV_0;          // ADC clock divider - 000b = Divide by 1
  ADCCTL1 |= ADCSSEL_0;         // ADC clock MODCLK
  ADCCTL1 |= ADCCONSEQ_0;       // ADC conversion sequence 00b = Single-channel single-conversion
// ADCCTL1 & ADCBUSY identifies a conversion is in process

// ADCCTL2 Register
  ADCCTL2 = 0;                  // Reset
  ADCCTL2 |= ADCPDIV0;          // ADC pre-divider 00b = Pre-divide by 1
  ADCCTL2 |= ADCRES_2;          // ADC resolution 10b = 12 bit (14 clock cycle conversion time)
  ADCCTL2 &= ~ADCDF;            // ADC data read-back format 0b = Binary unsigned.
  ADCCTL2 &= ~ADCSR;            // ADC sampling rate 0b = ADC buffer supports up to 200 ksps
  
// ADCMCTL0 Register
  ADCMCTL0 |= ADCSREF_0;        // VREF - 000b = {VR+ = AVCC and VR– = AVSS }
  ADCMCTL0 |= ADCINCH_2;        // V_THUMB (0x20) Pin 5 A5
  ADCIE |= ADCIE0;              // Enable ADC conv complete interrupt
  ADCCTL0 |= ADCENC;            // ADC enable conversion.
  ADCCTL0 |= ADCSC;             // ADC start conversion.
  
  P2OUT |= IR_LED;
}

#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void){
  switch(__even_in_range(ADCIV,ADCIV_ADCIFG)){
  case ADCIV_NONE:
    break;
  case ADCIV_ADCOVIFG:          // When a conversion result is written to the ADCMEM0
                                // before its previous conversion result was read.
    break;
  case ADCIV_ADCTOVIFG:         // ADC conversion-time overflow
    break;
  case ADCIV_ADCHIIFG:          // Window comparator interrupt flags
    break;
  case ADCIV_ADCLOIFG:          // Window comparator interrupt flag
    break;
  case ADCIV_ADCINIFG:          // Window comparator interrupt flag
    break;
  case ADCIV_ADCIFG:            // ADCMEM0 memory register with the conversion result
    ADCCTL0 &= ~ADCENC;         // Disable ENC bit.
    switch (ADC_Channel++){
    case 0x00:                                  // Channel A2 Interrupt
      ADCMCTL0 &= ~ADCINCH_2;                   // Disable Last channel A2
      ADCMCTL0 |= ADCINCH_3;                    // Enable Next channel A3
      ADC_Left_Detect = ADCMEM0;                // Move result into Global
      ADC_Left_Detect = ADC_Left_Detect >> 2;   // Divide the result by 4
      if (ADC_Left_Detect < 110) {
        ADC_Left_Detect = ADC_Left_Detect + 20;
      }
      else if (ADC_Left_Detect >= 150) {
        ADC_Left_Detect = ADC_Left_Detect - 115;
      }
      adc_line = LEFT;
      break;
    case 0x01:
      ADCMCTL0 &= ~ADCINCH_3;                           // Disable Last channel A3
      ADCMCTL0 |= ADCINCH_5;                            // Enable Next channel A5
      ADC_Right_Detect = ADCMEM0;                       // Move result into Global
      ADC_Right_Detect = ADC_Right_Detect >> 2;         // Divide the result by 4
      //ADC_Right_Detect = ADC_Right_Detect;
      if (ADC_Left_Detect >= 150) {
        ADC_Right_Detect = ADC_Right_Detect + 115;
      }
      adc_line = RIGHT;
      break;
    case 0x02:
      ADCMCTL0 &= ~ADCINCH_5;                           // Disable Last channel A5
      ADCMCTL0 |= ADCINCH_2;                            // Enable Next channel A2
      ADC_Thumb_Detect = ADCMEM0;                       // Move result into Global
      ADC_Thumb_Detect = ADC_Thumb_Detect >> 2;         // Divide the result by 4
      //HEXtoBCD(ADC_Thumb_Detect);                       // Convert result to String
      ADC_Channel=0;
      break;
    default:
      break;
}
ADCCTL0 |= ADCENC; // Enable Conversions
//ADCCTL0 |= ADCSC; // Start next sample
  default:
    break;
}
}

void line_move(void) {
  switch(travel_count) {
    case 0:                             // 1 second delay
      motion_state();                   // DELAY
      if (fr_event <= 40) {
        pwm_wheels_off();
      }
      else if (fr_event == 41) {
        fr_event = 0;
        travel_count++;
      }
      break;
    case 1:                               // Turn over line
      motion_state();                     // Turning
      if (fr_event <= 50) {
        pwm_wheels_ccw();
      }
      else if (fr_event == 51) {
        pwm_wheels_off();
        fr_event = 0;
        travel_count++;
      }
      break;
    case 2:
      motion_state();                   // DELAY
      if (fr_event <= 40) {
        pwm_wheels_off();
      }
      else if (fr_event == 41) {
        fr_event = 0;
        travel_count++;
      }
      break;
    case 3:                               // Move forward to Line
      motion_state();                     // Intercept
      if (fr_event <= 750) {
        pwm_wheels_on();
      }
      else if (fr_event == 751) {
        pwm_wheels_off();
        fr_event = 0;
        travel_count++;
      }
      break;
    case 4:                               // Stop 3-5 seconds
      motion_state();                     // Waiting
      if (fr_event <= 40) {
        pwm_wheels_off();
      }
      else if (fr_event == 41) {
        fr_event = 0;
        travel_count++;
      }
      break;
    case 5:                               // Circle twice
      motion_state();                     // Circling
      if (fr_event <= 450) {
          pwm_wheels_turn(Right_Turn_Speed, Left_Turn_Speed);
      }
      else if (fr_event == 451) {
        pwm_wheels_off();
        fr_event = 0;
        travel_count++;
      }
      break;
    case 6:
      motion_state();                     // Waiting
        if (fr_event <= 40) {
          pwm_wheels_off();
        }
        else if (fr_event == 41) {
          fr_event = 0;
          travel_count++;
        }
        break;
    case 7:                               // Circle twice
      motion_state();                     // Circling
      if (fr_event <= 1200) {
          pwm_wheels_turn(Right_Turn_Speed, Left_Turn_Speed);
      }
      else if (fr_event == 1201) {
        pwm_wheels_off();
        fr_event = 0;
        travel_count++;
      }
      break;
    case 8:
    motion_state();                     // Waiting
        if (fr_event <= 40) {
          pwm_wheels_off();
        }
        else if (fr_event == 41) {
          fr_event = 0;
          travel_count++;
        }
        break;
    case 9:                               // Turn towards outside circle
      motion_state();                     // Turning
      if (fr_event <= 30) {
        pwm_wheels_ccw();
      }
      else if (fr_event == 31) {
        pwm_wheels_off();
        fr_event = 0;
        travel_count++;
      }
      break;
    case 10:
      motion_state();                     // Waiting
      if (fr_event <= 40) {
        pwm_wheels_off();
      }
      else if (fr_event == 41) {
        fr_event = 0;
        travel_count++;
      }
      break;
    case 11:                               // Move outside circle
      motion_state();                     // Forward
      if (fr_event <= 180) {
        pwm_wheels_on();
      }
      else if (fr_event == 181) {
        pwm_wheels_off();
        fr_event = 0;
        travel_count++;
      }
      break;
    case 12:
      motion_state();                     // Waiting
      if (fr_event <= 40) {
        pwm_wheels_off();
      }
      else if (fr_event == 41) {
        fr_event = 0;
        travel_count++;
      }
      break;
    case 13:                               // Reset
      fr_event = 0;                       // Stopped
      sw_start = READY;
      //travel_count = 0;
      break;
    }
}

void displayTimer(int display_timer) {
  if (travel_count != 13) {
    int d_timer = display_timer >> 1;
    int value;
    value = 0;
    display_char[0] = '0';
    while (d_timer > 999){
      d_timer = d_timer - 1000;
      value = value + 1;
      display_char[0] = 0x30 + value;
    }
    value = 0;
    display_char[1] = '0';
    while (d_timer > 99){
      d_timer = d_timer - 100;
      value = value + 1;
      display_char[1] = 0x30 + value;
    }
    value = 0;
    display_char[2] = '0';
    while (d_timer > 9){
      d_timer = d_timer - 10;
      value = value + 1;
      display_char[2] = 0x30 + value;
    }
    display_char[3] = '.';
    display_char[4] = 0x30 + d_timer;
    display_char[5] = 0;
    
    strcpy(display_line[3], "          ");
    strcpy(display_line[3], display_char);
  }
}

void pwm_wheels_on(void) {
  RIGHT_REVERSE_SPEED = 0;
  LEFT_REVERSE_SPEED = 0;
  RIGHT_FORWARD_SPEED = HALF_SPEED_RIGHT;
  LEFT_FORWARD_SPEED = HALF_SPEED_LEFT;
}

void pwm_wheels_fw(void) {
  RIGHT_REVERSE_SPEED = 0;
  LEFT_REVERSE_SPEED = 0;
  RIGHT_FORWARD_SPEED = RIGHT_SPEED;
  LEFT_FORWARD_SPEED = LEFT_SPEED;
}

void pwm_wheels_off(void) {
  RIGHT_FORWARD_SPEED = 0;
  LEFT_FORWARD_SPEED = 0;
  RIGHT_REVERSE_SPEED = 0;
  LEFT_REVERSE_SPEED = 0;
}

void pwm_wheels_reverse(void) {
  RIGHT_FORWARD_SPEED = 0;
  LEFT_FORWARD_SPEED = 0;
  RIGHT_REVERSE_SPEED = RIGHT_SPEED;
  LEFT_REVERSE_SPEED = LEFT_SPEED;
}

void pwm_wheels_cw(void) {
  RIGHT_FORWARD_SPEED = 0;
  LEFT_REVERSE_SPEED = 0;
  RIGHT_REVERSE_SPEED = HALF_SPEED_RR;
  LEFT_FORWARD_SPEED = HALF_SPEED_FL; 
}

void pwm_wheels_ccw(void) {
  LEFT_FORWARD_SPEED = 0;
  RIGHT_REVERSE_SPEED = 0;
  LEFT_REVERSE_SPEED = HALF_SPEED_RL;
  RIGHT_FORWARD_SPEED = HALF_SPEED_FR;
}

void pwm_wheels_turn(int Right_Turn_Speed, int Left_Turn_Speed) {
  RIGHT_REVERSE_SPEED = 0;
  LEFT_REVERSE_SPEED = 0;
  RIGHT_FORWARD_SPEED = Right_Turn_Speed;
  LEFT_FORWARD_SPEED = Left_Turn_Speed;
}