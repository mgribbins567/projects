#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include <string.h>
#include  "ports.h"

extern volatile unsigned int usb_rx_ring_wr;
extern volatile unsigned int usb_rx_ring_rd;
extern volatile unsigned int usb_tx_ring_wr;
extern volatile unsigned int usb_tx_ring_rd;
extern volatile char USB_Char_Rx[SMALL_RING_SIZE];
extern volatile char USB_Char_Tx[SMALL_RING_SIZE];
extern volatile unsigned int usb_rx_ring_wr2;
extern volatile unsigned int usb_rx_ring_rd2;
extern volatile unsigned int usb_tx_ring_wr2;
extern volatile unsigned int usb_tx_ring_rd2;
extern volatile char USB_Char_Rx2[SMALL_RING_SIZE];
extern volatile char USB_Char_Tx2[SMALL_RING_SIZE];
extern char pb_index;
extern char pb_index2;
extern char output_chars[SMALL_RING_SIZE];
extern char output_chars2[SMALL_RING_SIZE];
extern char PC_Transmit_UCA1;
extern char PC_Transmit_UCA0;
extern char command_port[18];
extern char command_sync[21];
extern char command_status[12];
extern char tx_done;
extern unsigned int command_count;
extern int error_check;


#pragma vector = EUSCI_A0_VECTOR

__interrupt void eUSCI_A0_ISR(void){
unsigned int temp;
char temp_char;
//unsigned int count;
switch(__even_in_range(UCA0IV,0x08)){
  case 0:                                               // Vector 0 - no interrupt
    break;
  case 2:                                               // Vector 2 – RXIFG
    PC_Transmit_UCA0 = ON;
    
    temp = usb_rx_ring_wr++;
    
    temp_char = UCA0RXBUF;
    if (temp_char != 0x1B && temp_char != 0x0D) {
      USB_Char_Rx[temp] = temp_char;                      // RX -> USB_Char_Rx character
      UCA1TXBUF = temp_char;
    }
    
    if (usb_rx_ring_wr >= (sizeof(USB_Char_Rx))){
      usb_rx_ring_wr = BEGINNING;                       // Circular buffer back to beginning
    }

  // code for Receive
    break;
  case 4:                                               // Vector 4 – TXIFG
    //while(!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = output_chars[pb_index];                 // Transmit Current Indexed value
    output_chars[pb_index++] = NULL;                    // Null Location of Transmitted value
    if(output_chars[pb_index] == NULL){                 // Is the next pb_index location NULL - End of Command
      tx_done = ON;
      UCA0IE &= ~UCTXIE;                                // Disable TX interrupt
    }
    

    break;

  // Code for Transmit
  default: break;
  }
}
//------------------------------------------------------------------------------

#pragma vector = EUSCI_A1_VECTOR

__interrupt void eUSCI_A1_ISR(void){
unsigned int temp;
char temp_char;
switch(__even_in_range(UCA1IV,0x08)){
  case 0:                                               // Vector 0 - no interrupt
    break;
  case 2:                                               // Vector 2 – RXIFG
    temp = usb_rx_ring_wr2++;
    
    temp_char = UCA1RXBUF;
    if (temp_char != 0x1B && temp_char != 0x0D) {
      USB_Char_Rx2[temp] = temp_char;                      // RX -> USB_Char_Rx character
      UCA0TXBUF = temp_char;
    }

    if (usb_rx_ring_wr2 >= (sizeof(USB_Char_Rx2))){
      usb_rx_ring_wr2 = BEGINNING;                       // Circular buffer back to beginning
    }
  // code for Receive
    break;
  case 4:                                               // Vector 4 – TXIFG

    UCA1TXBUF = output_chars2[pb_index2];               // Transmit Current Indexed value
    output_chars2[pb_index2++] = NULL;                  // Null Location of Transmitted value
    if(output_chars2[pb_index2] == NULL){               // Is the next pb_index location NULL - End of Command
      UCA1IE &= ~UCTXIE;                                // Disable TX interrupt
    }
    break;

  // Code for Transmit
  default: 
    
    break;
  }
}
//------------------------------------------------------------------------------



//Init_Serial_UCA0(); // Initialize Serial Port for USB
void Init_Serial_UCA0(void){
  int i;
  for(i=0; i<SMALL_RING_SIZE; i++){
    USB_Char_Rx[i] = 0x00;                      // USB Rx Buffer
  }
  usb_rx_ring_wr = BEGINNING;
  usb_rx_ring_rd = BEGINNING;
  for(i=0; i<LARGE_RING_SIZE; i++){             // May not use this
    USB_Char_Tx[i] = 0x00;                      // USB Tx Buffer
  }
  usb_tx_ring_wr = BEGINNING;
  usb_tx_ring_rd = BEGINNING;
  // Configure UART 0
  UCA0CTLW0 = 0;                        // Use word register
  UCA0CTLW0 |= UCSWRST;                 // Set Software reset enable
  UCA0CTLW0 |= UCSSEL__SMCLK;           // Set SMCLK as fBRCLK
  UCA0CTLW0 &= ~UCMSB; // MSB, LSB select
  UCA0CTLW0 &= ~UCSPB; // UCSPB = 0(1 stop bit) OR 1(2 stop bits)
  UCA0CTLW0 &= ~UCPEN; // No Parity
  UCA0CTLW0 &= ~UCSYNC;
  UCA0CTLW0 &= ~UC7BIT;
  UCA0CTLW0 |= UCMODE_0;

  // 115,200 Baud Rate
  
  UCA0BRW = 4;                         
  // UCA0MCTLW = UCSx concatenate UCFx concatenate UCOS16;
  // UCA0MCTLW = 0x55 concatenate 1 concatenate 1;
  UCA0MCTLW = 0x5551;
  UCA0CTLW0 &= ~UCSWRST;               // Set Software reset enable
  UCA0IE |= UCRXIE;                     // Enable RX interrupt
  //UCA0TXBUF = 0x00;
  //UCA0IE |= UCTXIE;
}

void Init_Serial_UCA1(void){
  int i;
  for(i=0; i<SMALL_RING_SIZE; i++){
    USB_Char_Rx2[i] = 0x00;                      // USB Rx Buffer
  }
  usb_rx_ring_wr2 = BEGINNING;
  usb_rx_ring_rd2 = BEGINNING;
  for(i=0; i<LARGE_RING_SIZE; i++){             // May not use this
    USB_Char_Tx2[i] = 0x00;                      // USB Tx Buffer
  }
  usb_tx_ring_wr2 = BEGINNING;
  usb_tx_ring_rd2 = BEGINNING;
  // Configure UART 0
  UCA1CTLW0 = 0;                        // Use word register
  UCA1CTLW0 |= UCSWRST;                 // Set Software reset enable
  UCA1CTLW0 |= UCSSEL__SMCLK;           // Set SMCLK as fBRCLK
  UCA1CTLW0 &= ~UCMSB; // MSB, LSB select
  UCA1CTLW0 &= ~UCSPB; // UCSPB = 0(1 stop bit) OR 1(2 stop bits)
  UCA1CTLW0 &= ~UCPEN; // No Parity
  UCA1CTLW0 &= ~UCSYNC;
  UCA1CTLW0 &= ~UC7BIT;
  UCA1CTLW0 |= UCMODE_0;

  // 115,200 Baud Rate
  
  UCA1BRW = 4;                        
  // UCA0MCTLW = UCSx concatenate UCFx concatenate UCOS16;
  // UCA0MCTLW = 0x55 concatenate 1 concatenate 1;
  UCA1MCTLW = 0x5551;
  UCA1CTLW0 &= ~ UCSWRST;               // Set Software reset enable
  UCA1IE |= UCRXIE;                     // Enable RX interrupt
  //UCA1IE |= UCTXIE;
}

void Init_Arrays(void) {
  
  int i;
  command_port[0] = 'A';
  command_port[1] = 'T';
  command_port[2] = '+';
  command_port[3] = 'N';
  command_port[4] = 'S';
  command_port[5] = 'T';
  command_port[6] = 'C';
  command_port[7] = 'P';
  command_port[8] = '=';
  command_port[9] = '5';
  command_port[10] = '0';
  command_port[11] = '4';
  command_port[12] = '8';
  command_port[13] = ',';
  command_port[14] = '1';
  command_port[15] = 0x0D;
  command_port[16] = 0x0A;
  
  command_sync[0] = 'A';
  command_sync[1] = 'T';
  command_sync[2] = '+';
  command_sync[3] = 'W';
  command_sync[4] = 'S';
  command_sync[5] = 'Y';
  command_sync[6] = 'N';
  command_sync[7] = 'C';
  command_sync[8] = 'I';
  command_sync[9] = 'N';
  command_sync[10] = 'T';
  command_sync[11] = 'R';
  command_sync[12] = 'L';
  command_sync[13] = '=';
  command_sync[14] = '6';
  command_sync[15] = '5';
  command_sync[16] = '5';
  command_sync[17] = '3';
  command_sync[18] = '5';
  command_sync[19] = 0x0D;
  command_sync[20] = 0x0A; 

  switch(command_count) {
    case 0:
      for (i = 0; i < 17; i++) {
        output_chars[i] = command_port[i];
      }
      pb_index = 0;
      UCA0IE |= UCTXIE;
      UCA0TXBUF = output_chars[0];
      command_count = 1;
      break;
    case 1:
      if (tx_done == ON) {
        command_count = 2;
        tx_done = OFF;
      }
      break;
    case 2:
      for (i=0; i < 21; i++) {
        output_chars[i] = command_sync[i];
      }
      pb_index = 0;
      UCA0IE |= UCTXIE;
      UCA0TXBUF = output_chars[0];
      command_count = 3;
      break;
    case 3:
      if (tx_done == ON) {
        command_count = 4;
        tx_done = OFF;
      }
      break;
  case 4:
    if (error_check != 6) {
      command_count = 5;
    }
  }
  
  
}