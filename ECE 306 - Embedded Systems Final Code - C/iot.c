#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include <string.h>
#include  "ports.h"

extern char display_line[4][11];
extern volatile unsigned char update_display;
extern volatile char USB_Char_Rx[SMALL_RING_SIZE];
extern volatile char USB_Char_Rx2[SMALL_RING_SIZE];
extern char output_chars[SMALL_RING_SIZE];
extern char output_chars2[SMALL_RING_SIZE];
extern char output_ch[SMALL_RING_SIZE];
extern char PC_Transmit_UCA0;
extern char PC_Transmit_UCA1;
extern char tx_start;
extern char tx_check;
extern char tx_start2;
extern char tx_check2;
extern char pb_index;
extern char pb_index2;
extern volatile unsigned char update_display;
extern volatile unsigned int usb_rx_ring_wr;
extern volatile unsigned int usb_rx_ring_rd;
extern volatile unsigned int usb_rx_ring_wr2;
extern volatile unsigned int usb_rx_ring_rd2;
extern volatile unsigned int usb_tx_ring_wr2;
extern volatile unsigned int usb_tx_ring_rd2;
extern unsigned int time_hundred;
extern unsigned int time_tens;
extern unsigned int time_ones;
extern char command_direction;
extern char init_clear;
extern char ip_output[10];
extern char ip_output2[10];
extern int ip_found;
extern unsigned int travel_counter;
extern unsigned int time_drive;
extern char init_drive;
extern int ssid_found;
extern char ssid_output[10];
extern char command_issued;
extern char command_progress;
extern int command_location;
extern char command_block;
extern char direction_detect;
extern int time_count;
extern int arrive_check;
extern int location_inc;
extern int blcheck_stop;
extern int error_check;
extern unsigned int command_count;
extern char timer_start;




void iot_module(void) {
  char temp_char;
  if (command_block == OFF) {
  if (PC_Transmit_UCA0 == ON) {                 //Tx in UCA1
    while (usb_rx_ring_rd != usb_rx_ring_wr) {
      temp_char = USB_Char_Rx[usb_rx_ring_rd];
      if (temp_char == '!' && command_issued == OFF) {
        output_ch[0] = temp_char;
        command_issued = ON;
        timer_start = ON;
      }
      if (command_issued == ON && direction_detect == OFF) {
        switch(temp_char) {
        case '9':
          output_ch[1] = temp_char;
          break;
        case '8':
          output_ch[2] = temp_char;
          break;
        case '0':
          output_ch[3] = temp_char;
          break;
        case '6':
          output_ch[4] = temp_char;
          break;
        case 'F':
          output_ch[5] = temp_char;
          command_direction = FORWARD;
          direction_detect = ON;
          break;
        case 'B':
          output_ch[5] = temp_char;
          command_direction = BACKWARDS;
          direction_detect = ON;
          break;
        case 'R':
          output_ch[5] = temp_char;
          command_direction = RIGHT;
          direction_detect = ON;
          break;
        case 'L':
          output_ch[5] = temp_char;
          command_direction = LEFT;
          direction_detect = ON;
          break;
        }
      }
      else if (direction_detect == ON && command_issued == ON && time_count == 0) {
      output_ch[6] = temp_char;
      time_count = 1;
      switch(temp_char) {
        case '0':
          time_hundred = 0;
          break;
        case '1':
          time_hundred = 100;
          break;
        case '2':
          time_hundred = 200;
          break;
        case '3':
          time_hundred = 300;
          break;
        case '4':
          time_hundred = 400;
          break;
        case '5':
          time_hundred = 500;
          break;
        case '6':
          time_hundred = 600;
          break;
        case '7':
          time_hundred = 700;
          break;
        case '8':
          time_hundred = 800;
          break;
        case '9':
          time_hundred = 900;
          break;
      }
      }
      else if (direction_detect == ON && command_issued == ON && time_count == 1) {
      output_ch[7] = temp_char;
      time_count = 2;
      switch(temp_char) {
        case '0':
          time_tens = 0;
          break;
        case '1':
          time_tens = 10;
          break;
        case '2':
          time_tens = 20;
          break;
        case '3':
          time_tens = 30;
          break;
        case '4':
          time_tens = 40;
          break;
        case '5':
          time_tens = 50;
          break;
        case '6':
          time_tens = 60;
          break;
        case '7':
          time_tens = 70;
          break;
        case '8':
          time_tens = 80;
          break;
        case '9':
          time_tens = 90;
          break;
      }
      }
      else if (direction_detect == ON && command_issued == ON && time_count == 2) {
      output_ch[8] = temp_char;
      time_count = 3;
      switch(temp_char) {
        case '0':
          time_ones = 0;
          break;
        case '1':
          time_ones = 1;
          break;
        case '2':
          time_ones = 2;
          break;
        case '3':
          time_ones = 3;
          break;
        case '4':
          time_ones = 4;
          break;
        case '5':
          time_ones = 5;
          break;
        case '6':
          time_ones = 6;
          break;
        case '7':
          time_ones = 7;
          break;
        case '8':
          time_ones = 8;
          break;
        case '9':
          time_ones = 9;
          break;
      }
      command_block = ON;
      }
      
      else if (temp_char == '1' && ip_found == 0) {
        ip_output[0] = temp_char;
        usb_rx_ring_rd++;
        if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
          usb_rx_ring_rd = BEGINNING;                       
        }
        temp_char = USB_Char_Rx[usb_rx_ring_rd];
        ip_output[1] = temp_char;
        if (temp_char == '9') {
          usb_rx_ring_rd++;
          if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
            usb_rx_ring_rd = BEGINNING;                       
          }
          temp_char = USB_Char_Rx[usb_rx_ring_rd];
          ip_output[2] = temp_char;
          if (temp_char == '2') {
            usb_rx_ring_rd++;
            if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
              usb_rx_ring_rd = BEGINNING;                       
            }
            temp_char = USB_Char_Rx[usb_rx_ring_rd];
            ip_output[3] = temp_char;
            if (temp_char == '.') {
              usb_rx_ring_rd++;
              if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
                usb_rx_ring_rd = BEGINNING;                       
              }
              temp_char = USB_Char_Rx[usb_rx_ring_rd];
              ip_output[4] = temp_char;
              usb_rx_ring_rd+=1;
              if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
                usb_rx_ring_rd = BEGINNING;                       
              }
              temp_char = USB_Char_Rx[usb_rx_ring_rd];
              ip_output[5] = temp_char;
              usb_rx_ring_rd+=1;
              if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
                usb_rx_ring_rd = BEGINNING;                       
              }
              ip_found = 1;
            }
          }
        }
      }
      if (temp_char == '8' && ip_found == 1) {
        ip_output[6] = temp_char;
        usb_rx_ring_rd+=1;
        if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
          usb_rx_ring_rd = BEGINNING;                       
        }
        temp_char = USB_Char_Rx[usb_rx_ring_rd];
        if (temp_char == '.') {
          ip_found = 2;
          ip_output[7] = temp_char;
          usb_rx_ring_rd+=1;
          if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
            usb_rx_ring_rd = BEGINNING;                       
          }
          temp_char = USB_Char_Rx[usb_rx_ring_rd];
          if (temp_char != ':') {
            ip_output2[0] = temp_char;
            usb_rx_ring_rd+=1;
            if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
              usb_rx_ring_rd = BEGINNING;                       
            }
            temp_char = USB_Char_Rx[usb_rx_ring_rd];
            if (temp_char != ':') {
              ip_output2[1] = temp_char;
            }
          }
        }
      }
      if (ip_found == 2) {
        ip_found = 3;
        usb_rx_ring_rd+=1;
        if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
          usb_rx_ring_rd = BEGINNING;                       
        }
        temp_char = USB_Char_Rx[usb_rx_ring_rd];
          if (temp_char != ':') {
            ip_output2[2] = temp_char;
            usb_rx_ring_rd+=1;
            if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
              usb_rx_ring_rd = BEGINNING;                       
            }
            temp_char = USB_Char_Rx[usb_rx_ring_rd];
            if (temp_char != ':') {
              ip_output2[3] = temp_char;
              usb_rx_ring_rd+=1;
              if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
                usb_rx_ring_rd = BEGINNING;                       
              }
              temp_char = USB_Char_Rx[usb_rx_ring_rd];
              ip_found = 10;
              if (temp_char != ':') {
                ip_output2[4] = temp_char;
                usb_rx_ring_rd+=1;
                if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
                  usb_rx_ring_rd = BEGINNING;                       
                }
                temp_char = USB_Char_Rx[usb_rx_ring_rd];
                if (temp_char != ':') {
                  ip_output2[5] = temp_char;
                  usb_rx_ring_rd+=1;
                  if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
                    usb_rx_ring_rd = BEGINNING;                       
                  }
                  temp_char = USB_Char_Rx[usb_rx_ring_rd];
                  if (temp_char != ':') {
                    ip_output2[6] = temp_char;
                    usb_rx_ring_rd+=1;
                    if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
                      usb_rx_ring_rd = BEGINNING;                       
                    }
                    temp_char = USB_Char_Rx[usb_rx_ring_rd];
                    if (temp_char != ':') {
                      ip_output2[7] = temp_char;
                      usb_rx_ring_rd+=1;
                      if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
                        usb_rx_ring_rd = BEGINNING;                       
                      }
                      temp_char = USB_Char_Rx[usb_rx_ring_rd];
                    }
                  }
                }
              }
            }
          }
        }
      
      if (temp_char == 'A' && arrive_check == 0 && ip_found == 11) {
        arrive_check = 1;
      }
      else if (arrive_check >= 1) {
        switch(temp_char) {
        case 'R':
          if (arrive_check < 3) {
            arrive_check++;
          }
          else {
            arrive_check = 0;
          }
          break;
        case 'I':
          arrive_check++;
          break;
        case 'V':
          arrive_check++;
          break;
        case 'E':
          arrive_check++;
          break;
        case 'D':
          arrive_check++;
          break;
        default:
          arrive_check = 0;
          break;
        }
      }
      if (temp_char == 'B' && blcheck_stop == 0 && ip_found == 11) {
        blcheck_stop = 1;
      }
      else if (blcheck_stop >= 1) {
        switch(temp_char) {
        case 'L':
          blcheck_stop++;
          break;
        case 'S':
          blcheck_stop++;
          break;
        case 'T':
          blcheck_stop++;
          break;
        case 'O':
          blcheck_stop++;
          break;
        case 'P':
          blcheck_stop++;
          break;
        default:
          blcheck_stop = 0;
          break;
        }
      }
      if (temp_char == 'E' && error_check == 0 && ip_found == 10) {
        error_check = 1;
      }
      else if (error_check >= 1) {
        switch(temp_char) {
          case 'R':
            if (error_check != 3) {
              error_check++;
            }
            else {
              error_check = 0;
            }
            break;
          case 'O':
            error_check++;
            break;
        case ':':
          if (error_check == 5) {
            error_check++;
          }
          else {
            error_check = 0;
          }
          break;
        default: 
          error_check = 0; 
          break;
        }
      }
      
      usb_rx_ring_rd++;
      if (usb_rx_ring_rd >= (sizeof(USB_Char_Rx))){
        usb_rx_ring_rd = BEGINNING;                       
      }
    }
    PC_Transmit_UCA0 = OFF;
  }
}
}

void iot_commands(void) {
  if (command_issued == ON) {
    switch(command_direction) {
      case FORWARD:
        strcpy(display_line[0], output_ch);
        update_display = 1;
        if (init_drive == ON) {
          time_drive = time_hundred + time_tens + time_ones;
          //time_drive = time_drive;
          pwm_wheels_fw();
          travel_counter = 0;
          init_drive = OFF;
        }
        if (travel_counter == time_drive) {
          pwm_wheels_off();
          command_direction = NONE;
          command_issued = OFF;
          command_block = OFF;
          time_count = 0;
          direction_detect = OFF;
          init_drive = ON;
          travel_counter = 0;
          strcpy(display_line[0], "          ");
          update_display = 1;
        }
        break;
      case BACKWARDS:
        strcpy(display_line[0], output_ch);
        update_display = 1;
        if (init_drive == ON) {
          time_drive = time_hundred + time_tens + time_ones;
          //time_drive = time_drive;
          pwm_wheels_reverse();
          travel_counter = 0;
          init_drive = OFF;
        }
        if (travel_counter == time_drive) {
          pwm_wheels_off();
          command_direction = NONE;
          command_issued = OFF;
          command_block = OFF;
          time_count = 0;
          direction_detect = OFF;
          init_drive = ON;
          travel_counter = 0;
          strcpy(display_line[0], "          ");
          update_display = 1;
        }
        break;
      case RIGHT:
        strcpy(display_line[0], output_ch);
        update_display = 1;
        if (init_drive == ON) {
          time_drive = time_hundred + time_tens + time_ones;
          time_drive = time_drive >> 2;
          pwm_wheels_ccw();
          travel_counter = 0;
          init_drive = OFF;
        }
        if (travel_counter == time_drive) {
          pwm_wheels_off();
          command_direction = NONE;
          command_issued = OFF;
          command_block = OFF;
          time_count = 0;
          direction_detect = OFF;
          init_drive = ON;
          travel_counter = 0;
          strcpy(display_line[0], "          ");
          update_display = 1;
        }
        break;
      case LEFT:
        strcpy(display_line[0], output_ch);
        update_display = 1;
        if (init_drive == ON) {
          time_drive = time_hundred + time_tens + time_ones;
          time_drive = time_drive >> 2;
          pwm_wheels_cw();
          travel_counter = 0;
          init_drive = OFF;
          command_progress = LEFT;
        }
        if (travel_counter == time_drive) {
          pwm_wheels_off();
          command_direction = NONE;
          command_issued = OFF;
          command_block = OFF;
          time_count = 0;
          direction_detect = OFF;
          init_drive = ON;
          travel_counter = 0;
          strcpy(display_line[0], "          ");
          update_display = 1;
        }
        break;
    }
  }
}



void USCI_A0_transmit(void) {
  if (output_chars[pb_index] == NULL) {
    pb_index = 0;
    UCA0IE &= ~UCTXIE;    
  }
}
void USCI_A1_transmit(void) {
  if (output_chars2[pb_index2] == NULL) {
    pb_index2 = 0;
    UCA1IE |= UCTXIE;
  }
}