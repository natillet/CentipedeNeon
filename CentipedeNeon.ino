// Works with Centipede Shield or MCP23017 on Arduino I2C port

#include "CentipedeNeon.h"
#include "CentipedeMidi.h"
#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <MIDI.h>

volatile int sw0_pos = 0;
volatile int sw1_pos = 0;
volatile int sw2_pos = 0;
volatile int sw3_pos = 0;
volatile int sw4_pos = 0;
volatile int sw5_pos = 0;
volatile int swLR_pos = 0;
volatile bool switch_programs = false;
 
void setup()
{
  Wire.begin(); // start I2C
  midi_start();
 
  CS.initialize(); // set all registers to default
 
  CS.portMode(0, 0b0000000000000000); // set all pins on chip 0 to output (0 to 15)
  CS.portMode(1, 0b0000000000000000); // set all pins on chip 1 to output (16 to 31)
  CS.portMode(2, 0b0000000000000000); // set all pins on chip 2 to output (32 to 47)
  CS.portMode(3, 0b0000000000000000); // set all pins on chip 3 to output (48 to 63)
 
  //TWBR = 12; // uncomment for 400KHz I2C (on 16MHz Arduinos)
  
  //Button/Interrupt Setup
  pinMode(BTN, INPUT);
  digitalWrite(BTN, HIGH);
#ifdef UPLOAD_FROM_ARDUINODROID
  attachInterrupt(1, BtnISR, FALLING);
#else
  attachInterrupt(digitalPinToInterrupt(BTN), BtnISR, FALLING);
#endif //UPLOAD_FROM_ARDUINODROID
  
  //Switch Setup
//  pinMode(SWITCH0, INPUT);
//  pinMode(SWITCH1, INPUT);
//  pinMode(SWITCH2, INPUT);
  pinMode(SWITCH3, INPUT);
  pinMode(SWITCH4, INPUT);
  pinMode(SWITCH5, INPUT);
  pinMode(SWITCH_LR, INPUT);
//  digitalWrite(SWITCH0, HIGH);
//  digitalWrite(SWITCH1, HIGH);
//  digitalWrite(SWITCH2, HIGH);
  digitalWrite(SWITCH3, HIGH);
  digitalWrite(SWITCH4, HIGH);
  digitalWrite(SWITCH5, HIGH);
  digitalWrite(SWITCH_LR, HIGH);
  
//  //read program switches
//  sw0_pos = digitalRead(SWITCH0);
//  sw1_pos = digitalRead(SWITCH1);
//  sw2_pos = digitalRead(SWITCH2);
  
//  active_program = (display_t)((sw2_pos << 2) | (sw1_pos << 1) | sw0_pos);
  
  programSwitchDebounce = millis();
  
  sei();                  //Enable global interrupts
}
 

void loop()
{
  //read ADC
  delay_ms = analogRead(ADC0);
  delay_ms += 100; //don't let it go too fast
  
  if (switch_programs)
  {
    switch_programs = false;
    long long timeSinceLastSwitch = millis() - programSwitchDebounce;
    
    if ((timeSinceLastSwitch > PROGRAM_SWITCH_DEBOUNCE) || (timeSinceLastSwitch < 0))
    {
      if (active_program < MAX_DISPLAY)
      {
        active_program = (display_t)(((int)active_program) + 1);
      }
      else
      {
        active_program = (display_t)0;
      }
      programSwitchDebounce = millis();
    }
  }
  
  //read non-program switches
  sw3_pos = digitalRead(SWITCH3);
  sw4_pos = digitalRead(SWITCH4);
  sw5_pos = digitalRead(SWITCH5);
  swLR_pos = digitalRead(SWITCH_LR);
  
  if (0 == swLR_pos)
  {
    displayDirection = LEFT;
  }
  else
  {
    displayDirection = RIGHT;
  }
  
  global_x = (((sw3_pos << 1) | sw4_pos) << 1) + 2;
  global_y = (sw5_pos << 1) + 2;
  
  //choose program to display
  switch(active_program)
  {
    case ALL_BLINK:
      allblink();
      active_channel = 1;
      break;
    case WAVE_X_ON_Y_OFF:
      wave(global_x, global_y, displayDirection);
      active_channel = 2;
      break;
    case STEP_X_ON_Y_OFF:
      stepping(global_x, global_y, displayDirection);
      active_channel = 3;
      break;
    case STACK:
      stack(displayDirection);
      active_channel = 4;
      delay_modifier = -20;
      break;
    case RANDOM_X_ON:
      rand(global_x);
      active_channel = 5;
      break;
    case HALVES_WAVE_1_ON_LR:
      halves_wave_1_lr(displayDirection);
      active_channel = 6;
      break;
    case HALVES_WAVE_1_ON_IO:
      halves_wave_1_io(displayDirection);
      active_channel = 7;
      break;
    case PING_PONG_1_ON:
      ping_pong_1_on();
      active_channel = 8;
      break;
    case MAX_DISPLAY: //fall through
    default:
      active_program = ALL_BLINK;
      allblink();
      active_channel = 1;
      break;
  }

  //midi call
  midi_sequence(port0, port1, port2, port3);

  //send lights
  CS.portWrite(0, port0);
  CS.portWrite(1, port1);
  CS.portWrite(2, port2);
  CS.portWrite(3, port3);
  
  delay(delay_ms + delay_modifier);
}

//// BUTTON INTERRUPT ////
// Interrupt Service Routine attached to INT0 vector
void BtnISR(void)
{
  switch_programs = true;
}


