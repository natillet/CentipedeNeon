//Functionality for conveying Midi in time with the lights
#include "CentipedeMidi.h"

MIDI_CREATE_DEFAULT_INSTANCE();

CentipedeMidi::CentipedeMidi()
{
//  MIDI_CREATE_DEFAULT_INSTANCE();
}

void CentipedeMidi::midi_start()
{
  MIDI.begin();
}

void CentipedeMidi::midi_channel_switch()
{
  for (int i = 0; i < MAX_LIGHTS; i++)
  {
    MIDI.sendNoteOff(i+pitch_offset,0,active_channel);
  }
}

void CentipedeMidi::midi_sequence(unsigned int port0, unsigned int port1, unsigned int port2, unsigned int port3)
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

  //first turn off all the notes that should end
  for (int i = 0; i < MAX_LIGHTS; i++)
  {
    if (prev_midi_state[i] != midi_state[i])
    {
      if (0 == midi_state[i])
      {
        MIDI.sendNoteOff(i+pitch_offset,0,active_channel);
        prev_midi_state[i] = midi_state[i];
      }
    }
  }

  //turn on all the notes that should be start
  for (int i = 0; i < MAX_LIGHTS; i++)
  {
    if (prev_midi_state[i] != midi_state[i])
    {
      if (1 == midi_state[i])
      {
        MIDI.sendNoteOn(i+pitch_offset,active_velocity,active_channel);
        prev_midi_state[i] = midi_state[i];
      }
    }
  }

}

