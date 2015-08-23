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

#define ADC0 0

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

//prototypes after enums
void wave(int x_in, int y_in, displayDirection_t dir_in);
void stepping(int x_in, int y_in, displayDirection_t dir_in);
void stack(displayDirection_t dir_in);
void rand(int x_in);
void halves_wave_1_lr(displayDirection_t dir_in);
void halves_wave_1_io(displayDirection_t dir_in);

//Potentiometer sets speed
//Button for direction
//Button for display program
//X, Y adjust?

Centipede CS; // create Centipede object
const int MAX_LIGHTS = 64;

//Globals
int delay_ms = 100;
display_t active_program = STACK; //ALL_BLINK;
displayDirection_t displayDirection = RIGHT;
int global_x = 2;
int global_y = 2;

 
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
  delay_ms = analogRead(ADC0);
  delay_ms += 25; //don't let it go too fast
  
  //choose program to display
  switch(active_program)
  {
    case ALL_BLINK:
      allblink();
      break;
    case WAVE_X_ON_Y_OFF:
      wave(global_x, global_y, displayDirection);
      break;
    case STEP_X_ON_Y_OFF:
      stepping(global_x, global_y, displayDirection);
      break;
    case STACK:
      stack(displayDirection);
      break;
    case RANDOM_X_ON:
      rand(global_x);
      break;
    case HALVES_WAVE_1_ON_LR:
      halves_wave_1_lr(displayDirection);
      break;
    case HALVES_WAVE_1_ON_IO:
      halves_wave_1_io(displayDirection);
      break;
    case PING_PONG_1_ON:
      ping_pong_1_on();
      break;
    case MAX_DISPLAY: //fall through
    default:
      active_program = ALL_BLINK;
      break;
  }
}

//// DISPLAY PROGRAMS ////
void allblink(void)
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
  static unsigned int snake0 = 0;
  static unsigned int snake1 = 0;
  static unsigned int snake2 = 0;
  static unsigned int snake3 = 0;
  static int x = 0;
  static int y = 0;
  static bool turnOn = true;
  static displayDirection_t dir = LEFT;
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
      
      if (x >= x_in)
      {
        turnOn = false;
        x = 0;
      }
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
      
      if (y >= y_in)
      {
        turnOn = true;
        y = 0;
      }
    }
  }
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms);
}

void stepping(int x_in, int y_in, displayDirection_t dir_in)
{
  static unsigned int snake0 = 0;
  static unsigned int snake1 = 0;
  static unsigned int snake2 = 0;
  static unsigned int snake3 = 0;
  static int x = 0;
  static int y = 0;
  static bool turnOn = true;
  static displayDirection_t dir = LEFT;
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
    for (x = 0; x < x_in; x++)
    {
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
    turnOn = false;
  }
  else
  {
    for (y = 0; y < y_in; y++)
    {
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
    turnOn = true;
  }
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms);
}

void stack(displayDirection_t dir_in)
{
  static unsigned long long snake = 0;
  static unsigned long long snake_slider = 0;
  static unsigned long long snake_stack = 0;
  static int level = 0;      //starts at 0, where stack level is none, then 1-64 are stack slots
  static int slide = 63;    //0-63
  static bool sliding = false;
  static displayDirection_t dir = LEFT;
  unsigned int snake0 = 0;
  unsigned int snake1 = 0;
  unsigned int snake2 = 0;
  unsigned int snake3 = 0;
  if (dir != dir_in)
  {
    dir = dir_in;
    //reset snake so it doesn't look wonky
    snake = 0;
    slide = 63;
    level = 0;
  }
  
  if (sliding)
  {
    if (LEFT == dir)
    {
      snake_slider = (snake_slider << 1);  //slide the light left
    }
    else
    {
      snake_slider = (snake_slider >> 1);  //slide the light right
    }
    slide--;  //count down the slide to the level (which counts up toward the slide)
    if (slide <= level)  //if slide hits the notch above the current level, update the stack to that level (level 0 is all off)
    {
      level++;  //update the stack to the next level
      if (LEFT == dir)
      {
        snake_stack = (snake_stack >> 1) + 0x08000000000000000LL;  //grow the stack
      }
      else
      {
        snake_stack = (snake_stack << 1) + 0x01;  //grow the stack
      }
      sliding = false;  //begin the next sliding light
      if (level >= 64)  //if this was the last level, reset
      {
        snake = 0;
        snake_stack = 0;
        slide = 63;
        level = 0;
      }
    }
  }
  else
  {
    slide = 63;
    sliding = true;
    if (LEFT == dir)
    {
      snake_slider = 0x01;  //start the next slide
    }
    else
    {
      snake_slider = 0x08000000000000000LL;  //start the next slide
    }
  }
  
  snake = snake_slider | snake_stack;
  snake0 = snake & 0x000000000000ffffLL;
  snake1 = (snake & 0x00000000ffff0000LL) >> 16;
  snake2 = (snake & 0x0000ffff00000000LL) >> 32;
  snake3 = (snake & 0xffff000000000000LL) >> 48;
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms-20);
}

void rand(int x_in)
{
  int snake_temp = 0;
  int snake0 = 0;
  int snake1 = 0;
  int snake2 = 0;
  int snake3 = 0;
  int i = 0;
  
  while (i < x_in)
  {
      int quadrant = random(4);
      snake_temp = 1 << random(16);
      switch(quadrant)
      {
        case 0:
          snake0 |= snake_temp;
          break;
        case 1:
          snake1 |= snake_temp;
          break;
        case 2:
          snake2 |= snake_temp;
        case 3:
        default:
          snake3 |= snake_temp;
          break;
      }
    i++;
  }
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms*10);
}

void halves_wave_1_lr(displayDirection_t dir_in)
{
  static int count = 0;
  static const int half = MAX_LIGHTS >> 1;  //divide MAX_LIGHTS in half
  static const int quarter = MAX_LIGHTS >> 2;  //divide MAX_LIGHTS in quarter
  static unsigned int snake0 = 0;
  static unsigned int snake1 = 0;
  static unsigned int snake2 = 0;
  static unsigned int snake3 = 0;
  static displayDirection_t dir = LEFT;
  if (dir != dir_in)
  {
    dir = dir_in;
    //reset snake so it doesn't look wonky
    snake0 = 0;
    snake1 = 0;
    snake2 = 0;
    snake3 = 0;
  }
  
  if (LEFT == dir)
  {
    if (0 == count)
    {
      snake0 = 1;
      snake1 = 0;
      snake2 = 1;
      snake3 = 0;
    }
    else if (quarter == count)
    {
      snake0 = 0;
      snake1 = 1;
      snake2 = 0;
      snake3 = 1;
    }
    else
    {
      snake0 = snake0 << 1;
      snake1 = snake1 << 1;
      snake2 = snake2 << 1;
      snake3 = snake3 << 1;
    }
  }
  else
  {
    if (0 == count)
    {
      snake0 = 0;
      snake1 = 0x08000;
      snake2 = 0;
      snake3 = 0x08000;
    }
    else if (quarter == count)
    {
      snake0 = 0x08000;
      snake1 = 0;
      snake2 = 0x08000;
      snake3 = 0;
    }
    else
    {
      snake0 = snake0 >> 1;
      snake1 = snake1 >> 1;
      snake2 = snake2 >> 1;
      snake3 = snake3 >> 1;
    }
  }
  
  if (count < half)
  {
    count++;
  }
  else
  {
    count = 0;
  }
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms);
}

void halves_wave_1_io(displayDirection_t dir_in)
{
  static int count = 0;
  static const int half = MAX_LIGHTS >> 1;  //divide MAX_LIGHTS in half
  static const int quarter = MAX_LIGHTS >> 2;  //divide MAX_LIGHTS in quarter
  static unsigned int snake0 = 0;
  static unsigned int snake1 = 0;
  static unsigned int snake2 = 0;
  static unsigned int snake3 = 0;
  static displayDirection_t dir = LEFT;  //left will by in, right will be out
  if (dir != dir_in)
  {
    dir = dir_in;
    //reset snake so it doesn't look wonky
    snake0 = 0;
    snake1 = 0;
    snake2 = 0;
    snake3 = 0;
  }
  
  if (LEFT == dir)
  {
    if (0 == count)
    {
      snake0 = 1;
      snake1 = 0;
      snake2 = 0;
      snake3 = 0x08000;
    }
    else if (quarter == count)
    {
      snake0 = 0;
      snake1 = 1;
      snake2 = 0x08000;
      snake3 = 0;
    }
    else
    {
      snake0 = snake0 << 1;
      snake1 = snake1 << 1;
      snake2 = snake2 >> 1;
      snake3 = snake3 >> 1;
    }
  }
  else
  {
    if (0 == count)
    {
      snake0 = 0;
      snake1 = 0x08000;
      snake2 = 1;
      snake3 = 0;
    }
    else if (quarter == count)
    {
      snake0 = 0x08000;
      snake1 = 0;
      snake2 = 0;
      snake3 = 1;
    }
    else
    {
      snake0 = snake0 >> 1;
      snake1 = snake1 >> 1;
      snake2 = snake2 << 1;
      snake3 = snake3 << 1;
    }
  }
  
  if (count < half)
  {
    count++;
  }
  else
  {
    count = 0;
  }
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms);
}

void ping_pong_1_on(void)
{
  static int count = 0;
  static const int half = MAX_LIGHTS >> 1;  //divide MAX_LIGHTS in half
  static const int quarter = MAX_LIGHTS >> 2;  //divide MAX_LIGHTS in quarter
  static unsigned int snake0 = 0;
  static unsigned int snake1 = 0;
  static unsigned int snake2 = 0;
  static unsigned int snake3 = 0;
  static displayDirection_t dir = LEFT;
  
  if (LEFT == dir)
  {
    if (0 == count)
    {
      snake0 = 1;
      snake1 = 0;
      snake2 = 0;
      snake3 = 0;
    }
    else if (quarter == count)
    {
      snake0 = 0;
      snake1 = 1;
      snake2 = 0;
      snake3 = 0;
    }
    else if (half == count)
    {
      snake0 = 0;
      snake1 = 0;
      snake2 = 1;
      snake3 = 0;
    }
    else if ((half+quarter) == count)
    {
      snake0 = 0;
      snake1 = 0;
      snake2 = 0;
      snake3 = 1;
    }
    else
    {
      snake0 = snake0 << 1;
      snake1 = snake1 << 1;
      snake2 = snake2 << 1;
      snake3 = snake3 << 1;
    }
    count++;
  }
  else
  {
    if (MAX_LIGHTS == count)
    {
      snake0 = 0;
      snake1 = 0;
      snake2 = 0;
      snake3 = 0x08000;
    }
    else if ((half+quarter) == count)
    {
      snake0 = 0;
      snake1 = 0;
      snake2 = 0x08000;
      snake3 = 0;
    }
    else if (half == count)
    {
      snake0 = 0;
      snake1 = 0x08000;
      snake2 = 0;
      snake3 = 0;
    }
    else if (quarter == count)
    {
      snake0 = 0x08000;
      snake1 = 0;
      snake2 = 0;
      snake3 = 0;
    }
    else
    {
      snake0 = snake0 >> 1;
      snake1 = snake1 >> 1;
      snake2 = snake2 >> 1;
      snake3 = snake3 >> 1;
    }
    count--;
  }
  
  if (count >= MAX_LIGHTS)
  {
    dir = RIGHT;
    count = MAX_LIGHTS;
  }
  else if (count <= 0)
  {
    dir = LEFT;
    count = 0;
  }
  
  CS.portWrite(0, snake0);
  CS.portWrite(1, snake1);
  CS.portWrite(2, snake2);
  CS.portWrite(3, snake3);
  delay(delay_ms);
}

