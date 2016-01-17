//Functionality for conveying Midi in time with the lights
#include "CentipedeMidi.h"

MIDI_CREATE_DEFAULT_INSTANCE();

byte midi_state[MAX_LIGHTS] = {0};
byte prev_midi_state[MAX_LIGHTS] = {0};
byte active_channel = 1;
byte active_velocity = 64;

void midi_start()
{
  MIDI.begin();
}

void midi_sequence(unsigned int port0, unsigned int port1, unsigned int port2, unsigned int port3)
{
  for(int i = 0; i < 16; i++)
  {
    midi_state[i]    = port0 & 0x0001;
    midi_state[i+16]  = port1 & 0x0001;
    midi_state[i+32] = port2 & 0x0001;
    midi_state[i+48] = port3 & 0x0001;
    port0 = port0 >> 1;
    port1 = port1 >> 1;
    port2 = port2 >> 1;
    port3 = port3 >> 1;
  }

  for (int i = 0; i < MAX_LIGHTS; i++)
  {
    if (prev_midi_state[i] != midi_state[i])
    {
      if (midi_state[i] = 1)
      {
        MIDI.sendNoteOn(i,active_velocity,active_channel);
      }
      else
      {
        MIDI.sendNoteOff(i,0,active_channel);
      }
      prev_midi_state[i] = midi_state[i];
    }
  }

}

