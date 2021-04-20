//------------------------------------------------------------------------------
//
//  Description: This file contains the Main Routine - "While" Operating System
//
//
//  Jim Carlson
//  Jan 2018
//  Built with IAR Embedded Workbench Version: V4.10A/W32 (7.11.2)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include <string.h>
#include  "ports.h"

#define ALWAYS                  (1)
#define RESET_STATE             (0)
#define RED_LED              (0x01) // RED LED 0
#define GRN_LED              (0x40) // GREEN LED 1

// Function Prototypes
void main(void);
void Init_Conditions(void);
void Init_LEDs(void);

  // Global Variables
extern char display_line[4][11];
extern char *display[4];
unsigned char display_mode;
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
volatile unsigned int Time_Sequence;
volatile char one_time;
unsigned int Last_Time_Sequence;                // a variable to identify Time_Sequence has changed
unsigned int cycle_time;                        // is a new time base used to control making shapes
unsigned int time_change;                       // is an identifier that a change has occurred
unsigned volatile int debounce_count_sw1;
unsigned volatile int debounce_count_sw2;
unsigned int backlite_count;
unsigned char sw_event = READY;
unsigned int switch1_press;
unsigned int switch2_press;
unsigned char sw_press = OFF;
unsigned char sw_start;
unsigned int fr_event;
unsigned int travel_count = 0;
char adc_char[10];
char display_char[6];
unsigned int ADC_Channel;
unsigned int ADC_Left_Detect;
unsigned int ADC_Right_Detect;
unsigned int ADC_Thumb_Detect;
unsigned char adc_line;
unsigned char switch_count;
unsigned int Right_Turn_Speed = RIGHT_TURN;
unsigned int Left_Turn_Speed = LEFT_TURN;
unsigned int display_timer;
volatile unsigned int usb_rx_ring_wr;
volatile unsigned int usb_rx_ring_rd;
volatile unsigned int usb_tx_ring_wr;
volatile unsigned int usb_tx_ring_rd;
volatile char USB_Char_Rx[SMALL_RING_SIZE];     // May need to not be volatile
volatile char USB_Char_Tx[SMALL_RING_SIZE];
volatile unsigned int usb_rx_ring_wr2;
volatile unsigned int usb_rx_ring_rd2;
volatile unsigned int usb_tx_ring_wr2;
volatile unsigned int usb_tx_ring_rd2;
volatile char USB_Char_Rx2[SMALL_RING_SIZE];
volatile char USB_Char_Tx2[SMALL_RING_SIZE];
char pb_index = 1;
char pb_index2;
char output_chars[SMALL_RING_SIZE];
char output_ch[SMALL_RING_SIZE];
char output_chars2[SMALL_RING_SIZE];
char baud_init = ON;
char iot_res = OFF;
unsigned int time_hundred;
unsigned int time_tens;
unsigned int time_ones;
char command_direction;
char PC_Transmit_UCA0 = OFF;
char PC_Transmit_UCA1 = OFF;
unsigned int travel_counter;
unsigned int time_drive;
char init_drive = ON;
char command_port[18];
char tx_done = OFF;
unsigned int command_count = 0;
char array_res = OFF;
char ip_output[10];
char ip_output2[10];
int ip_found = 0;
int ssid_found = 0;
char ssid_output[10];
char command_sync[21];
char command_status[12];
char command_issued = OFF;
char command_progress = OFF;
int command_location;
char command_block = OFF;
char direction_detect = OFF;;
int time_count = 0;
int location_inc = 0;
char black_line_start = OFF;
char black_line_init = ON;
int arrive_check = 0;
int blcheck_stop = 0;
int error_check = 0;
int white_detect = WHITE_DETECT;
int black_detect = BLACK_DETECT;
char line_state;
char prev_state;
char full_off;
char timer_start = OFF;



void main(void){
//------------------------------------------------------------------------------
// Main Program
// This is the main routine for the program. Execution of code starts here.
// The operating system is Back Ground Fore Ground.
//
//------------------------------------------------------------------------------
// Disable the GPIO power-on default high-impedance mode to activate
// previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;
  Init_Ports();                        // Initialize Ports
  Init_Clocks();                       // Initialize Clock System
  Init_Conditions();                   // Initialize Variables and Initial Conditions
  //Init_Timers();                       // Initialize Timers
  Init_Timer_B0();
  Init_Timer_B3();
  Init_Serial_UCA0();
  Init_Serial_UCA1();
  //Init_ADC();
  Init_LCD();                          // Initialize LCD
  //Init_Arrays();
  //UCA0IE |= UCTXIE;
  //UCA1IE |= UCTXIE;
// Place the contents of what you want on the display, in between the quotes
// Limited to 10 characters per line
//

  strcpy(display_line[0], "WaitingInp");
  strcpy(display_line[1], "          ");
  strcpy(display_line[2], "          ");
  strcpy(display_line[3], "          ");
  update_display = 1;
  Display_Update(0,0,0,0);

//------------------------------------------------------------------------------
// Begining of the "While" Operating System
//------------------------------------------------------------------------------
  while(ALWAYS) {                      // Can the Operating system run
    
    if (iot_res == ON) {
      P5OUT |= IOT_RESET;
      iot_res = END;
      array_res = ON;
    }
    if (error_check == 6 && ip_found == 10) {
      command_count = 0;
      P1OUT |= RED_LED;
      error_check = 0;
    }
    else if (error_check > 6) {
      error_check = 0;
    }
    if (ip_found == 10) {
      strcpy(display_line[1], ip_output);
      strcpy(display_line[2], ip_output2);
      update_display = 1;
      
      if (command_count != 5) {
          Init_Arrays();
      }
      else {
        P1OUT &= ~RED_LED;
        ip_found = 11;
        
      }
    }
    iot_module();
    if (command_block == ON) {
      iot_commands();
    }
    if (arrive_check == 7) {
      location_inc++;
      switch(location_inc) {
        case 1:
          strcpy(display_line[0], "ARRIVED 01");
        break;
      case 2:
          strcpy(display_line[0], "ARRIVED 02");
        break;
      case 3:
          strcpy(display_line[0], "ARRIVED 03");
        break;
      case 4:
          strcpy(display_line[0], "ARRIVED 04");
        break;
      case 5:
          strcpy(display_line[0], "ARRIVED 05");
        break;
      case 6:
          strcpy(display_line[0], "ARRIVED 06");
        break;
      case 7:
          strcpy(display_line[0], "ARRIVED 07");
        break;
      case 8:
          strcpy(display_line[0], "ARRIVED 08");
          black_line_start = ON;
        break;
      }
      update_display = 1;
      arrive_check = 0;
    }
    
    if (black_line_start == ON) {
      if (black_line_init == ON) {
        black_line_init = OFF;
        Init_ADC();
      }
      line_move();
      black_line_navigate();
      
    }
    if (travel_count == 7 && blcheck_stop == 6) {
      pwm_wheels_off();
      travel_count++;
      fr_event = 0;
    }
    if (travel_count == 3 && ADC_Left_Detect >= black_detect) {
      pwm_wheels_off();
      travel_count++;
      fr_event = 0;
    }
    
    if (timer_start == ON) {
      displayTimer(display_timer);
      update_display = 1;
    }
      
    
    
             
      

 
    
    
    
    //if (adc_line == THUMB) {
      //HEXtoBCD(ADC_Thumb_Detect);
      //adc_line4();
    //}
    //else if (adc_line == LEFT) {
      //HEXtoBCD(ADC_Left_Detect);
      //displayTimer(display_timer);
      //update_display = 1;
      //adc_line4();
      //if ((switch1_press == 1) && (ADC_Left_Detect >= 225) && (stop_motion == NONE)) {  // Black line initially detected
        //if (switch2_press == 1) {
          //if ((travel_count == 1) || (travel_count == 4)) {
            //pwm_wheels_off();
            //fr_event = 0;
            //travel_count++;
            //if (travel_count == 4) {
              //stop_motion = OFF;
            //}
          //}
        //}
      //}
      
      
      //if ((travel_count == 5) && (ADC_Left_Detect >= 225) && (ADC_Right_Detect >= 225)) {
        //if (line_detect == W_LEFT) {
          //  Right_Turn_Speed = RIGHT_TURN;
            //Left_Turn_Speed = LEFT_TURN + 1000;
        //}
        //else if (line_detect == W_RIGHT) {
          //  Right_Turn_Speed = RIGHT_TURN;
            //Left_Turn_Speed = LEFT_TURN;
        //}
        //else {
          //  Right_Turn_Speed = RIGHT_TURN;
            //Left_Turn_Speed = LEFT_TURN;
        //}
            //line_detect = BLACK;
      //}
      //else if ((travel_count == 5) && (ADC_Left_Detect < 225) && (ADC_Right_Detect >= 225)) {
        //pwm_wheels_off();
        //Left_Turn_Speed = LEFT_TURN;
        //Right_Turn_Speed = RIGHT_TURN + INC_SPEED_R;
        //line_detect = W_LEFT;
      //}
      //else if ((travel_count == 5) && (ADC_Right_Detect < 225) && (ADC_Left_Detect >= 225)) {
        //pwm_wheels_off();
        //Right_Turn_Speed = LOW_SPEED;
        //Left_Turn_Speed = LEFT_TURN + INC_SPEED;
        //line_detect = W_RIGHT;
      //}
      //else if ((travel_count == 5) && (ADC_Right_Detect < 225) && (ADC_Left_Detect < 225)) {
        //if (line_detect == W_LEFT) {
          //Left_Turn_Speed = 0;
          //Right_Turn_Speed = RIGHT_TURN;
        //}
        //else if (line_detect == W_RIGHT) {
          //Right_Turn_Speed = 0;
          //Left_Turn_Speed = LEFT_TURN;
        //}
      //}
    //}
    //else if (adc_line == RIGHT) {
      //HEXtoBCD(ADC_Right_Detect);
      //adc_line4();
    //}
    //if (sw_start == ONGOING) {
      //line_move();
    //}
       
    
    //switch(event){
      //case STRAIGHT: // Straight
        //Run_Straight();
        //break; //
      //case CIRCLE: // Circle
        //Run_CircleCW();
        //break; //
      //case FIGURE8: // Figure 8
        //Run_F8();
        //break; //
      //case TRIANGLE: // Triangle
        //Run_Tri();
        //break; //
      //default: break;
    //}
  }
//------------------------------------------------------------------------------
}


void motion_state(void) {
  if (travel_count == 0) {
    strcpy(display_line[0], " BL DELAY ");
    update_display = 1;
  }
  else if (travel_count == 1) {
    strcpy(display_line[0], " BL START ");
    update_display = 1;
  }
  else if (travel_count == 2) {
    strcpy(display_line[0], "  BLSTOP  ");
    update_display = 1;
  }
  else if (travel_count == 3) {
    strcpy(display_line[0], " INTERCEPT  ");
    update_display = 1;
  }
  else if (travel_count == 4) {
    strcpy(display_line[0], "  BLSTOP  ");
    update_display = 1;
  }
  else if (travel_count == 5) {
    strcpy(display_line[0], " BLTRAVEL ");
    update_display = 1;
  }
  else if (travel_count == 6) {
    strcpy(display_line[0], "  BLSTOP  ");
    update_display = 1;
  }
  else if (travel_count == 7) {
    strcpy(display_line[0], " BLCIRCLE ");
    update_display = 1;
  }
  else if (travel_count == 8) {
    strcpy(display_line[0], "  BLSTOP  ");
    update_display = 1;
  }
  else if (travel_count == 9) {
    strcpy(display_line[0], "  BLEXIT  ");
    update_display = 1;
  }
  else if (travel_count == 10) {
    strcpy(display_line[0], "  BLSTOP  ");
    update_display = 1;
  }
}
  


