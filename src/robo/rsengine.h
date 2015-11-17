#ifndef _SPEECH_CORE_
#define _SPEECH_CORE_

#include "english.h"

#define numVocab sizeof(s_vocab)/sizeof(VOCAB)
#define numPhoneme sizeof(s_phonemes)/sizeof(PHONEME)

// Speak an english phrase
void say(char * );

// Speak a list of phonemes
void speak(char * );

// Set default pitch
void setPitch(uint8_t pitch);
#endif
