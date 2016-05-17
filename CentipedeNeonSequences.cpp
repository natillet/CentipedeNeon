#include "CentipedeNeon.h"

int delay_ms = 100;
display_t active_program = STACK;
displayDirection_t displayDirection = RIGHT;
int global_x = 6;
int global_y = 2;
unsigned long programSwitchDebounce = 0;
int delay_modifier = 0;
const int change_cycles = 1680; //60s/min * 4min * 7changes/s = 4min of changes

unsigned int port0 = 0;
unsigned int port1 = 0;
unsigned int port2 = 0;
unsigned int port3 = 0;

//// DISPLAY PROGRAMS ////
bool allblink(void)
{
  static bool complete = false;
  static unsigned int cycles = 0;
  static bool turnOn = true;
  if (turnOn)
  {
    port0 = 0xFFFF;
    port1 = 0xFFFF;
    port2 = 0xFFFF;
    port3 = 0xFFFF;
    turnOn = false;
  }
  else
  {
    port0 = 0;
    port1 = 0;
    port2 = 0;
    port3 = 0;
    turnOn = true;
  }
  cycles++;
  if (cycles >= change_cycles)
  {
    cycles = 0;
    complete = true;
  }

  if (complete)
  {
    complete = false;
    return true;
  }
  else
  {
    return complete;
  }
}

bool wave(int x_in, int y_in, displayDirection_t dir_in)
{
  static bool complete = false;
  static unsigned int cycles = 0;
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
      
      if (y >= y_in)
      {
        turnOn = true;
        y = 0;
      }
    }
    else
    {
      turnOn = true;
      y = 0;
    }
  }
  
  port0 = snake0;
  port1 = snake1;
  port2 = snake2;
  port3 = snake3;
  
  cycles++;
  if (cycles >= change_cycles)
  {
    cycles = 0;
    complete = true;
  }

  if (complete)
  {
    complete = false;
    return true;
  }
  else
  {
    return complete;
  }
}

bool stepping(int x_in, int y_in, displayDirection_t dir_in)
{
  static bool complete = false;
  static unsigned int cycles = 0;
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
  
  port0 = snake0;
  port1 = snake1;
  port2 = snake2;
  port3 = snake3;
  
  cycles++;
  if (cycles >= change_cycles)
  {
    cycles = 0;
    complete = true;
  }

  if (complete)
  {
    complete = false;
    return true;
  }
  else
  {
    return complete;
  }
}

bool stack(displayDirection_t dir_in)
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
    snake_slider = 0;
    snake_stack = 0;
    slide = 63;
    level = 0;
    sliding = false;
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
  
  port0 = snake0;
  port1 = snake1;
  port2 = snake2;
  port3 = snake3;

  if (((LEFT == dir) && (0x0FFFF == snake0)) || ((RIGHT == dir) && (0x0FFFF == snake3)))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool rand(int x_in)
{
  static bool complete = false;
  static unsigned int cycles = 0;
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
  
  port0 = snake0;
  port1 = snake1;
  port2 = snake2;
  port3 = snake3;
  
  cycles++;
  if (cycles >= change_cycles)
  {
    cycles = 0;
    complete = true;
  }

  if (complete)
  {
    complete = false;
    return true;
  }
  else
  {
    return complete;
  }
}

bool halves_wave_1_lr(displayDirection_t dir_in)
{
  static bool complete = false;
  static unsigned int cycles = 0;
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
  
  port0 = snake0;
  port1 = snake1;
  port2 = snake2;
  port3 = snake3;
  
  cycles++;
  if ((cycles >= change_cycles) && (1 == port0))
  {
    cycles = 0;
    complete = true;
  }

  if (complete)
  {
    complete = false;
    return true;
  }
  else
  {
    return complete;
  }
}

bool halves_wave_1_io(displayDirection_t dir_in)
{
  static bool complete = false;
  static unsigned int cycles = 0;
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
  
  port0 = snake0;
  port1 = snake1;
  port2 = snake2;
  port3 = snake3;
  
  cycles++;
  if ((cycles >= change_cycles) && (1 == port0))
  {
    cycles = 0;
    complete = true;
  }

  if (complete)
  {
    complete = false;
    return true;
  }
  else
  {
    return complete;
  }
}

bool ping_pong_1_on(void)
{
  static bool complete = false;
  static unsigned int cycles = 0;
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
  
  port0 = snake0;
  port1 = snake1;
  port2 = snake2;
  port3 = snake3;
  
  cycles++;
  if ((cycles >= change_cycles) && (1 == port0))
  {
    cycles = 0;
    complete = true;
  }

  if (complete)
  {
    complete = false;
    return true;
  }
  else
  {
    return complete;
  }
}

