#include "ports.h"

#define RESET_STATE             (0)
#define ALWAYS                  (1)
#define ORIGINAL             (0x00) //
#define RED_LED              (0x01) // RED LED 0
#define GRN_LED              (0x40) // 6 GREEN LED
#define SW2                  (0x08) // 2.3 SW2
#define SW1                  (0x02) // 4.1 SW1
#define Time_Sequence_Rate   (50)

#define S250    (0xFA) //
#define S200    (0xC8) //
#define S150    (0x96) //
#define S100    (0x64) //
#define S50     (0x32) //
#define S30     (0x1E) //
#define S25     (0x1A) //
#define S20     (0x16) //
#define S10     (0x0A) //
#define S3      (0x03) //
#define S2      (0x02) //
#define S1      (0x01) //
#define S0      (0x00) //

#define USE_GPIO             (0x00)
#define USE_SMCLK            (0x01)

//Defines for clock timers
#define DIVS (0x0030)
#define DIVS_L (0x0030)
#define DIVS0 (0x0010)
#define DIVS0_L (0x0010)
#define DIVS1 (0x0020)
#define DIVS1_L (0x0020)
#define DIVS_0 (0x0000) // /1
#define DIVS_1 (0x0010) // /2
#define DIVS_1_L (0x0010)
#define DIVS_2 (0x0020) // /4
#define DIVS_2_L (0x0020)
#define DIVS_3 (0x0030) // /8
#define DIVS_3_L (0x0030)
#define DIVS__1 (0x0000) // /1
#define DIVS__2 (0x0010) // /2
#define DIVS__2_L (0x0010)
#define DIVS__4 (0x0020) // /4
#define DIVS__4_L (0x0020)
#define DIVS__8 (0x0030) // /8
#define DIVS__8_L (0x0030)

//Defines for turning
#define NONE ('N')
#define STRAIGHT ('L')
#define CIRCLE ('C')
#define TRIANGLE ('T')
#define FIGURE8 ('F')
#define WAIT ('W')
#define START ('S')
#define RUN ('R')
#define END ('E')
#define WHEEL_COUNT_TIME (10)
#define WHEEL_COUNT_TIME_C (25)
#define RIGHT_COUNT_TIME (7)
#define LEFT_COUNT_TIME (8)
#define TRAVEL_DISTANCE_L (39)
#define TRAVEL_DISTANCE_R (38)
#define TRAVEL_DISTANCE_TU (25)
#define TRAVEL_DISTANCE_TR (10)
#define WAITING2START (50)

//Defines for Interrupts
#define P4PUD   (P4OUT)
#define P2PUD   (P2OUT)
#define TimerB0_0CCR_Vector             (Timer0_B0_VECTOR)
#define TimerB0_1_2_OF_CCR_Vector       (Timer0_B1_VECTOR)
#define TB0CCR0_INTERVAL                (25000)
#define TB0CCR1_INTERVAL                (25000)
#define TB0CCR2_INTERVAL                (25000)
#define TB0CCR3_INTERVAL                (25000)
#define READY ('G')
#define OFF ('O')
#define ON ('P')
#define DEBOUNCE ('D')
#define ONGOING ('I')
//#define Timer0_B1_VECTOR                ()
#define SWITCH1 ('A')
#define SWITCH2 ('B')

//Defines for PWM
#define RIGHT_FORWARD_SPEED     (TB3CCR1)
#define LEFT_FORWARD_SPEED      (TB3CCR2)
#define RIGHT_REVERSE_SPEED     (TB3CCR3)
#define LEFT_REVERSE_SPEED      (TB3CCR4)
#define WHEEL_OFF (0x00)
#define WHEEL_PERIOD (40000)
#define MAX_SPEED (39000)

#define LEFT_TURN (5800)
#define RIGHT_TURN (5500)
#define LEFT_TURN_LOW (4500)
#define RIGHT_TURN_LOW (4500)
#define INC_SPEED_L (10000)
#define INC_SPEED_L_BIG (2000)
#define INC_SPEED_L_FULL (8000)
#define INC_SPEED_R (3300)
#define INC_SPEED_R_BIG (3500)
#define INC_SPEED_R_FULL (3500)

#define RIGHT_SPEED (14000)
#define LEFT_SPEED (12000)
#define HALF_SPEED_LEFT (7500)
#define HALF_SPEED_RIGHT (7500)
#define HALF_SPEED_LEFT_TURN (32000)
#define HALF_SPEED_RIGHT_TURN (33600)
#define HALF_SPEED (15000)
#define HALF_SPEED_FR (12000)
#define HALF_SPEED_RR (12000)
#define HALF_SPEED_FL (10000)
#define HALF_SPEED_RL (10000)

//Defines for ADC
#define THUMB ('H')
#define RIGHT ('X')
#define LEFT ('Y')

//Defines for Circle
#define BLACK ('B')
#define WHITE ('W')
#define L_WHITE ('L')
#define R_WHITE ('R')
#define INSIDE ('I')
#define OUTSIDE ('O')
#define BLACK_DETECT (250)
#define WHITE_DETECT (110)

//Defines for eUSCI_A
#define BEGINNING (0)
#define SMALL_RING_SIZE (64)
#define LARGE_RING_SIZE (32)

#define FORWARD ('F')
#define BACKWARDS ('B')



