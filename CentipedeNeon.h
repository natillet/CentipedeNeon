//CentipedeNeon constants and such

#ifndef CentipedeNeon_h
#define CentipedeNeon_h

#include "Centipede.h"

#define UPLOAD_FROM_ARDUINODROID 0  //For uploading using Josh's phone, which doesn't have libraries and needs hardcoded values

#define ADC0 0  //analog pin of adc
//#define SWITCH0 0  //digital pin for program selector bit0 - program
//#define SWITCH1 1  //digital pin for program selector bit1 - program
//#define SWITCH2 2  //digital pin for program selector bit2 - program
                   //digital pin 3 in use for button, below
#define SWITCH3 4  //digital pin for program selector bit3 - count on
#define SWITCH4 5  //digital pin for program selector bit4 - count on
#define SWITCH5 6  //digital pin for program selector bit5 - count off
#define SWITCH_LR 7  //digital pin for display direction selector - left/right
#define BTN 3      //digital pin for interruptable button program selector (attachInterrupt only allows usage of digital pins 2 & 3)

typedef enum
{
  LEFT = 0,
  RIGHT
} displayDirection_t;

typedef enum
{
  ALL_BLINK = 0,
  WAVE_X_ON_Y_OFF,      //wave - groups of X travelling as a snake (Y blank between to get so far as to have a single X snake at a time)
  STEP_X_ON_Y_OFF,      //steps - group of X stepping to next group of X (Y blank between)
  STACK,                //stacking - one light travels to the end and stays, then another travels to the second to the end and stays, etc.
  RANDOM_X_ON,          //randomized - max of X on at a time
  HALVES_WAVE_1_ON_LR,  //two lights travel either left or right, half the length apart
  HALVES_WAVE_1_ON_IO,  //two lights travel either outward in or inward out
  PING_PONG_1_ON,       //one light travels left then right
  MAX_DISPLAY
} display_t;

const int MAX_LIGHTS = 64;
const int PROGRAM_SWITCH_DEBOUNCE = 500;

//Globals
extern Centipede CS; // create Centipede object
extern int delay_ms;
extern display_t active_program;
extern displayDirection_t displayDirection;
extern int global_x;
extern int global_y;
extern unsigned long programSwitchDebounce;
extern int delay_modifier;

//prototypes
void BtnISR(void);
void allblink();  //suggest remove
void wave(int x_in, int y_in, displayDirection_t dir_in);
void stepping(int x_in, int y_in, displayDirection_t dir_in);  //suggest remove
void stack(displayDirection_t dir_in);
void rand(int x_in);
void halves_wave_1_lr(displayDirection_t dir_in);
void halves_wave_1_io(displayDirection_t dir_in);
void ping_pong_1_on();

#endif
