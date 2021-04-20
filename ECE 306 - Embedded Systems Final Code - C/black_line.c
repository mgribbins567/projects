#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include <string.h>
#include  "ports.h"

extern unsigned int ADC_Left_Detect;
extern unsigned int ADC_Right_Detect;
extern unsigned int travel_count;
extern unsigned char adc_line;
extern unsigned int Right_Turn_Speed;
extern unsigned int Left_Turn_Speed;
extern char line_state;
extern char prev_state;
extern char full_off;
extern char config_white;
extern char config_black;
extern int white_detect;
extern int black_detect;

void black_line_navigate(void) {
  if (travel_count == 5 || travel_count == 7) {
        if (ADC_Right_Detect >= black_detect && ADC_Left_Detect >= black_detect) {
          line_state = BLACK;
        }
        //else if (ADC_Right_Detect < white_detect && ADC_Left_Detect < white_detect) {
          //line_state = WHITE;
        //}
        else if (ADC_Right_Detect >= black_detect && ADC_Left_Detect < black_detect) {
          line_state = L_WHITE;
        }
        else if (ADC_Right_Detect < black_detect && ADC_Left_Detect >= black_detect) {
          line_state = R_WHITE;
        }
        else {
          line_state = BLACK;
        }
      switch(line_state) {
        case BLACK:

            Left_Turn_Speed = LEFT_TURN;
            Right_Turn_Speed = RIGHT_TURN;

          break;
        case WHITE:
          //if (prev_state == L_WHITE) {          // Left sensor off, full off on inside
            //full_off = INSIDE;
            //Left_Turn_Speed = LEFT_TURN_LOW;
            //Right_Turn_Speed = RIGHT_TURN + INC_SPEED_R_FULL;
          //}
          //else if (prev_state == R_WHITE) {     // Right sensor off, full off on outside
            //full_off = OUTSIDE;
            //Left_Turn_Speed = LEFT_TURN + INC_SPEED_L_FULL;
            //Right_Turn_Speed = RIGHT_TURN_LOW; 
          //}
          //else {
            //Left_Turn_Speed = LEFT_TURN + INC_SPEED_L_FULL;
            //Right_Turn_Speed = RIGHT_TURN_LOW;
          //}
          break;
        case L_WHITE:
            Left_Turn_Speed = LEFT_TURN + INC_SPEED_L;
            Right_Turn_Speed = RIGHT_TURN_LOW;
            prev_state = L_WHITE;
          break;
        case R_WHITE:
            Left_Turn_Speed = LEFT_TURN_LOW;
            Right_Turn_Speed = RIGHT_TURN + INC_SPEED_R;
            prev_state = R_WHITE;
          break;
      }
  }
}