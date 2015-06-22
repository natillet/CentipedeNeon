// Example code for Centipede Library
// Works with Centipede Shield or MCP23017 on Arduino I2C port
 
#include <Wire.h>
#include "Centipede.h"
 
 
/* Available commands
  .digitalWrite([0...127], [LOW...HIGH]) - Acts like normal digitalWrite
  .digitalRead([0...127]) - Acts like normal digitalRead
  .pinMode([0...127], [INPUT...OUTPUT]) - Acts like normal pinMode
  .portWrite([0...7], [0...65535]) - Writes 16-bit value to one port (chip)
  .portRead([0...7]) - Reads 16-bit value from one port (chip)
  .portMode([0...7], [0...65535]) - Write I/O mask to one port (chip)
  .pinPullup([0...127], [LOW...HIGH]) - Sets pullup on input pin
  .portPullup([0...7], [0...65535]) - Sets pullups on one port (chip)
  .init() - Sets all registers to initial values
 
  Examples
  CS.init();
  CS.pinMode(0,OUTPUT);
  CS.digitalWrite(0, HIGH);
  int recpin = CS.digitalRead(0);
  CS.portMode(0, 0b0111111001111110); // 0 = output, 1 = input
  CS.portWrite(0, 0b1000000110000001); // 0 = LOW, 1 = HIGH
  int recport = CS.portRead(0);
  CS.pinPullup(1,HIGH);
  CS.portPullup(0, 0b0111111001111110); // 0 = no pullup, 1 = pullup
*/

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
  MAX_DISPLAY
} display_t;

//prototypes after enums
void wave(int x_in, int y_in, displayDirection_t dir_in);

//Potentiometer sets speed
//Button for direction
//Button for display program
//X, Y adjust?

Centipede CS; // create Centipede object
const int MAX_LIGHTS = 64;

int delay_ms = 100;
display_t active_program = WAVE_X_ON_Y_OFF; //ALL_BLINK;
displayDirection_t displayDirection = RIGHT;
int global_x = 1;
int global_y = 1;
 
void setup()
{
  Wire.begin(); // start I2C
 
  CS.initialize(); // set all registers to default
 
  CS.portMode(0, 0b0000000000000000); // set all pins on chip 0 to output (0 to 15)
  CS.portMode(1, 0b0000000000000000); // set all pins on chip 1 to output (16 to 31)
  CS.portMode(2, 0b0000000000000000); // set all pins on chip 2 to output (32 to 47)
  CS.portMode(3, 0b0000000000000000); // set all pins on chip 3 to output (48 to 63)
 
  //TWBR = 12; // uncomment for 400KHz I2C (on 16MHz Arduinos)
 
}
 

void loop()
{
  //read ADC
  
  //choose program to display
  switch(active_program)
  {
    case ALL_BLINK:
      blink();
      break;
    case WAVE_X_ON_Y_OFF:
      wave(global_x, global_y, displayDirection);
      break;
    case STEP_X_ON_Y_OFF:
      stepping(global_x, global_y);
      break;
    case STACK:
      break;
    case RANDOM_X_ON:
      break;
    case MAX_DISPLAY: //fall through
    default:
      active_program = ALL_BLINK;
      break;
  }
}

//// DISPLAY PROGRAMS ////
void blink(void)
{
  static bool turnOn = true;
  if (turnOn)
  {
    CS.portWrite(0, 0xFFFF);
    CS.portWrite(1, 0xFFFF);
    CS.portWrite(2, 0xFFFF);
    CS.portWrite(3, 0xFFFF);
    turnOn = false;
  }
  else
  {
    CS.portWrite(0, 0);
    CS.portWrite(1, 0);
    CS.portWrite(2, 0);
    CS.portWrite(3, 0);
    turnOn = true;
  }
  delay(delay_ms*10);
}

void wave(int x_in, int y_in, displayDirection_t dir_in)
{
  static int snake0 = 0;
  static int snake1 = 0;
  static int snake2 = 0;
  static int snake3 = 0;
  static int x = 0;
  static int y = 0;
  static bool turnOn = true;
  static displayDirection_t dir = RIGHT;
  if (dir != dir_in)
  {
    dir = dir_in;
    //reset snake so it doesn't look wonky
    snake0 = 0;
    snake1 = 0;
    snake2 = 0;
    snake3 = 0;
  }
  if (turnOn)
  {
    if (x < x_in)
    {
      x++;
      if (LEFT == dir)
      {
        snake3 = (snake3 << 1) + ((snake2 & 0x08000) == 0x08000);
        snake2 = (snake2 << 1) + ((snake1 & 0x08000) == 0x08000);
        snake1 = (snake1 << 1) + ((snake0 & 0x08000) == 0x08000);
        snake0 = (snake0 << 1) + 1;
      }
      else
      {
        snake0 = (snake0 >> 1) + ((snake1 & 0x01) << 15);
        snake1 = (snake1 >> 1) + ((snake2 & 0x01) << 15);
        snake2 = (snake2 >> 1) + ((snake3 & 0x01) << 15);
        snake3 = (snake3 >> 1) + 0x08000;
      }
    }
    else
    {
      turnOn = false;
      x = 0;
    }
  }
  else
  {
    if (y < y_in)
    {
      y++;
      if (LEFT == dir)
      {
        snake3 = (snake3 << 1) + ((snake2 & 0x08000) == 0x08000);
        snake2 = (snake2 << 1) + ((snake1 & 0x08000) == 0x08000);
        snake1 = (snake1 << 1) + ((snake0 & 0x08000) == 0x08000);
        snake0 = (snake0 << 1);
      }
      else
      {
        snake0 = (snake0 >> 1) + ((snake1 & 0x01) << 15);
        snake1 = (snake1 >> 1) + ((snake2 & 0x01) << 15);
        snake2 = (snake2 >> 1) + ((snake3 & 0x01) << 15);
        snake3 = (snake3 >> 1);
      }
    }
    else
    {
      turnOn = true;
      y = 0;
    }
  }
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms);
}

void stepping(int x_in, int y_in)
{
  static int snake0 = 0;
  static int snake1 = 0;
  static int snake2 = 0;
  static int snake3 = 0;
  static int x = 0;
  static int y = 0;
  static bool turnOn = true;
  if (turnOn)
  {
    for (x = 0; x < x_in; x++)
    {
      if (LEFT == displayDirection)
      {
        snake3 = (snake3 << 1) + ((snake2 & 0x080) >> 8);
        snake2 = (snake2 << 1) + ((snake1 & 0x080) >> 8);
        snake1 = (snake1 << 1) + ((snake0 & 0x080) >> 8);
        snake0 = (snake0 << 1) + 1;
      }
      else
      {
        snake0 = (snake0 >> 1) + ((snake1 & 0x01) << 8);
        snake1 = (snake1 >> 1) + ((snake2 & 0x01) << 8);
        snake2 = (snake2 >> 1) + ((snake3 & 0x01) << 8);
        snake3 = (snake3 >> 1) + 0x80;
      }
    }
    turnOn = false;
  }
  else
  {
    for (y = 0; y < y_in; y++)
    {
      if (LEFT == displayDirection)
      {
        snake3 = (snake3 << 1) + (snake2 & 0x80);
        snake2 = (snake2 << 1) + (snake1 & 0x80);
        snake1 = (snake1 << 1) + (snake0 & 0x80);
        snake0 = (snake0 << 1);
      }
      else
      {
        snake0 = (snake0 >> 1) + ((snake1 & 0x01) << 8);
        snake1 = (snake1 >> 1) + ((snake2 & 0x01) << 8);
        snake2 = (snake2 >> 1) + ((snake3 & 0x01) << 8);
        snake3 = (snake3 >> 1);
      }
    }
    turnOn = true;
  }
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms);
}


