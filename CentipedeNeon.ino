// ************************************************************
// CentipedeNeon is meant to control a set of 64 neon lights
// using a Centipede Shield to expand the number of I/O lines
// and a MIDI Shield to coordinate sounds with each light.
// Arduino: Uno
// Author: Elene Trull
// Centipede Shield on Arduino I2C port (A4 & A5)
//   http://docs.macetech.com/doku.php/centipede_shield
// MIDI Shield on Arduino UART port (D0 & D1)
// ************************************************************

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
CentipedeMidi CM; // create Midi object
Centipede CS; // create Centipede object
 
void setup()
{
  Wire.begin(); // start I2C
  CM.midi_start();
 
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
  pinMode(SWITCH3, INPUT);
  pinMode(SWITCH4, INPUT);
  pinMode(SWITCH5, INPUT);
  pinMode(SWITCH_LR, INPUT);
  digitalWrite(SWITCH3, HIGH);
  digitalWrite(SWITCH4, HIGH);
  digitalWrite(SWITCH5, HIGH);
  digitalWrite(SWITCH_LR, HIGH);
  
  programSwitchDebounce = millis();
  
  sei();                  //Enable global interrupts
}
 

void loop()
{
  //read ADC
  delay_ms = analogRead(ADC0);
  delay_ms += 50; //don't let it go too fast
  
  if (switch_programs)
  {
    switch_programs = false;
    long long timeSinceLastSwitch = millis() - programSwitchDebounce;
    
    if ((timeSinceLastSwitch > PROGRAM_SWITCH_DEBOUNCE) || (timeSinceLastSwitch < 0))
    {
      if (active_program < MAX_DISPLAY-1)
      {
        active_program = (display_t)(((int)active_program) + 1);
      }
      else
      {
        active_program = (display_t)0;
      }
      
      
      programSwitchDebounce = millis();
    }

    CM.midi_channel_switch();
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
      CM.active_channel = 1;
      break;
    case WAVE_X_ON_Y_OFF:
      global_x = 8;  //hardcoding for music
      global_y = 4;  //hardcoding for music
      displayDirection = LEFT;  //hardcoding for music
      wave(global_x, global_y, displayDirection);
      CM.active_channel = 2;
      break;
    case STEP_X_ON_Y_OFF:
      global_x = 8;  //hardcoding for music
      global_y = 4;  //hardcoding for music
      displayDirection = LEFT;  //hardcoding for music
      stepping(global_x, global_y, displayDirection);
      CM.active_channel = 3;
      break;
    case STACK:
      stack(displayDirection);
      CM.active_channel = 4;
      delay_modifier = -20;
      break;
    case RANDOM_X_ON:
      rand(global_x);
      CM.active_channel = 5;
      break;
    case HALVES_WAVE_1_ON_LR:
      halves_wave_1_lr(displayDirection);
      CM.active_channel = 6;
      break;
    case HALVES_WAVE_1_ON_IO:
      halves_wave_1_io(displayDirection);
      CM.active_channel = 7;
      break;
    case PING_PONG_1_ON:
      ping_pong_1_on();
      CM.active_channel = 8;
      break;
    case MAX_DISPLAY: //fall through
    default:
      active_program = WAVE_X_ON_Y_OFF; //ALL_BLINK;  //skip all blink
      allblink();
      CM.active_channel = 1;
      break;
  }

  //midi call
  CM.midi_sequence(port0, port1, port2, port3);

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


