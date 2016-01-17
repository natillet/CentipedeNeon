//CentipedeMidi constants and such
#ifndef CentipedeMidi_h
#define CentipedeMidi_h

#include <MIDI.h>
#include "CentipedeNeon.h"

extern byte midi_state[MAX_LIGHTS];
extern byte active_channel;

//prototypes
void midi_sequence(unsigned int port0, unsigned int port1, unsigned int port2, unsigned int port3);

#endif

