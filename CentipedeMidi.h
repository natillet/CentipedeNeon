//CentipedeMidi constants and such
#ifndef CentipedeMidi_h
#define CentipedeMidi_h

#include <MIDI.h>
#include "CentipedeNeon.h"

class CentipedeMidi
{
  private:
    byte midi_state[MAX_LIGHTS] = {0};
    byte prev_midi_state[MAX_LIGHTS] = {0};
    byte active_velocity = 64;
    const byte pitch_offset = 12;

  public:
    byte active_channel = 1;
    CentipedeMidi();
    void midi_start();
    void midi_channel_switch();
    void midi_sequence(unsigned int port0, unsigned int port1, unsigned int port2, unsigned int port3);
};

#endif

