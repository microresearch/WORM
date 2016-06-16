#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"


// license:BSD-3-Clause
// copyright-holders:Joseph Zbiciak,Tim Lindner
/**********************************************************************

    SP0256 Narrator Speech Processor emulation

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 28  OSC 2
                _RESET   2 |             | 27  OSC 1
           ROM DISABLE   3 |             | 26  ROM CLOCK
                    C1   4 |             | 25  _SBY RESET
                    C2   5 |             | 24  DIGITAL OUT
                    C3   6 |             | 23  Vdi
                   Vdd   7 |    SP0256   | 22  TEST
                   SBY   8 |             | 21  SER IN
                  _LRQ   9 |             | 20  _ALD
                    A8  10 |             | 19  SE
                    A7  11 |             | 18  A1
               SER OUT  12 |             | 17  A2
                    A6  13 |             | 16  A3
                    A5  14 |_____________| 15  A4

**********************************************************************/

/*
   GI SP0256 Narrator Speech Processor

   By Joe Zbiciak. Ported to MESS by tim lindner.

*/

/* Commencing ARM stripped port - with lap tests*/

// where are ROMS we can test with? sp0256 roms: sp0256-al2.ic1 (jupace), sp0256a-al2, sp0256-al2.bin, sp0256b-019.bin, dkspeech.rom

//const uint8_t spWHISKY[] __attribute__ ((section (".flash"))) = {

typedef unsigned char UINT8;
typedef signed char INT8;
typedef unsigned short uint16_t;
typedef unsigned short UINT16;
typedef signed short INT16;
typedef unsigned int UINT32;
typedef signed int INT32;



// al2.bin dumped here// now we use one from sauro.zip

// needs to start at 0x1000  and rom is till 0x10000 but only 2k...

// length is 2048 with is 2k or 0x1000

void bitrevbuff(UINT8 *buffer, unsigned int start, unsigned int length);

unsigned char m_rom[65536];

const unsigned char HAPPIER[7]={0x2d,4,4,4};

const unsigned char SILENT[7]={4,4,4,4,4,4,4};



const unsigned char m_roma[]= {0xE0, 0x7B, 0xE0, 0x7, 0xE0, 0x47, 0xE0, 0x27, 0xE0, 0x67, 0xE0, 0x97, 0xE8, 0x28, 0xE8, 0xFC, 0xE8, 0x32, 0xE8, 0xFA, 0xE8, 0x4E, 0xE8, 0x89, 0xE8, 0xB5, 0xE8, 0x5D, 0xE8, 0x4B, 0xE8, 0xF7, 0xE8, 0x3F, 0xE4, 0x8, 0xE4, 0xC4, 0xE4, 0xDC, 0xE4, 0xEE, 0xE4, 0x59, 0xE4, 0xD5, 0xE4, 0xFD, 0xE4, 0x33, 0xE4, 0xFB, 0xEC, 0xA8, 0xEC, 0x44, 0xEC, 0xDC, 0xEC, 0xCA, 0xEC, 0xBA, 0xEC, 0x56, 0xEC, 0x91, 0xEC, 0xC5, 0xEC, 0x9D, 0xEC, 0xF3, 0xEC, 0x8F, 0xE2, 0xE0, 0xE2, 0xE4, 0xE2, 0xDC, 0xE2, 0x5A, 0xE2, 0x26, 0xE2, 0xAE, 0xE2, 0xF1, 0xE2, 0x75, 0xE2, 0x63, 0xE2, 0x5B, 0xE2, 0x3F, 0xEA, 0x8C, 0xEA, 0x1A, 0xEA, 0x3E, 0xEA, 0xF1, 0xEA, 0x7B, 0xE6, 0xAC, 0xE6, 0xA, 0xE6, 0x16, 0xE6, 0x4E, 0xE6, 0x15, 0xE6, 0xBD, 0xE6, 0xA7, 0xEE, 0xDC, 0xEE, 0x6, 0xEE, 0x6E, 0xEE, 0x19, 0xE1, 0x0, 0xE1, 0x40, 0xE1, 0x20, 0xE1, 0x60, 0xE1, 0x10, 0xE1, 0x50, 0xE1, 0x30, 0xE1, 0x70, 0xE1, 0x8, 0xE1, 0x48, 0xE1, 0x28, 0xE1, 0x68, 0xE1, 0x18, 0xE1, 0x58, 0xE1, 0x38, 0xE1, 0x78, 0xE1, 0x4, 0xE1, 0x44, 0xE1, 0x24, 0xE1, 0x64, 0xE1, 0x14, 0xE1, 0x54, 0xE1, 0x34, 0xE1, 0x74, 0xE1, 0xC, 0xE1, 0x4C, 0xE1, 0x2C, 0xE1, 0x6C, 0xE1, 0x1C, 0xE1, 0x5C, 0xE1, 0x3C, 0xE1, 0x7C, 0x8, 0x0, 0x4, 0x0, 0xC, 0x0, 0x2, 0x0, 0xA, 0x0, 0x6, 0x0, 0xE, 0x0, 0x1, 0x0, 0x9, 0x0, 0x5, 0x0, 0xD, 0x0, 0x3, 0x0, 0xB, 0x0, 0x7, 0x0, 0xF, 0x0, 0xF1, 0x0, 0xF4, 0x0, 0xF7, 0x0, 0xFF, 0x0, 0x1D, 0xFF, 0x0, 0x10, 0x33, 0xE5, 0x96, 0xA9, 0xAF, 0x3F, 0x43, 0xB0, 0x64, 0xCA, 0xA3, 0xF6, 0x47, 0x55, 0xB4, 0xFE, 0x29, 0x8E, 0xDA, 0x1F, 0x77, 0x6D, 0x51, 0x75, 0xF4, 0x7E, 0xA9, 0xB3, 0xE2, 0x4F, 0xD5, 0x56, 0xFD, 0xA5, 0xDA, 0xCA, 0x7F, 0x16, 0x49, 0xFB, 0x7, 0x0, 0x10, 0x31, 0xEE, 0xD6, 0xED, 0xB3, 0xBF, 0x1A, 0xA2, 0x27, 0xAA, 0xCD, 0xF6, 0xCB, 0xB9, 0x5B, 0x52, 0xAD, 0xCD, 0x5F, 0x8A, 0xCD, 0xFF, 0x4A, 0xB5, 0x56, 0xFF, 0xA9, 0xD7, 0x7E, 0x1E, 0xE5, 0x56, 0xFE, 0xA7, 0x5A, 0xDA, 0x81, 0x14, 0x49, 0x3D, 0x0, 0x0, 0x18, 0x36, 0xFB, 0x56, 0x41, 0x4B, 0x91, 0xF8, 0x2C, 0x9D, 0x4C, 0x15, 0x0, 0xF4, 0x18, 0x23, 0xD, 0x0, 0x3A, 0x82, 0x1F, 0x6D, 0xB9, 0x84, 0x1, 0x18, 0x4, 0x84, 0x88, 0x15, 0x3, 0x0, 0xFD, 0x18, 0x24, 0x5, 0x0, 0x2A, 0x96, 0x7E, 0xE7, 0xD7, 0x84, 0x1, 0x50, 0x45, 0xE4, 0xEB, 0x3C, 0x3, 0x0, 0x18, 0x24, 0x15, 0x0, 0x29, 0x21, 0x3, 0x46, 0x9F, 0xE6, 0xDC, 0xF2, 0xA8, 0xD1, 0x11, 0x0, 0x6, 0xE0, 0x98, 0xD3, 0x94, 0x5B, 0x1D, 0x2C, 0x43, 0xFD, 0xA7, 0x74, 0x8B, 0x6E, 0x0, 0x18, 0x37, 0xCF, 0xD6, 0x80, 0x6, 0xF, 0xFF, 0x15, 0x9C, 0x2A, 0x74, 0xD2, 0x0, 0x92, 0xB4, 0x81, 0x14, 0x1E, 0x32, 0x3, 0x0, 0x1, 0x0, 0x20, 0x5F, 0x19, 0x0, 0x18, 0x35, 0xFB, 0x56, 0x81, 0x44, 0xD, 0xEB, 0x8F, 0xC6, 0xD, 0x14, 0x0, 0xF5, 0x18, 0x23, 0x1D, 0x40, 0x35, 0xA7, 0x23, 0x84, 0x9E, 0xA4, 0x2, 0x20, 0x46, 0x74, 0x4C, 0xA9, 0xCF, 0x2E, 0x78, 0x7, 0x9C, 0xE, 0x0, 0x18, 0x35, 0xC7, 0x96, 0xA7, 0x71, 0x39, 0xE, 0x1E, 0x64, 0x45, 0x66, 0xAA, 0x9A, 0xB8, 0xC7, 0x79, 0x5B, 0x52, 0x2D, 0xC5, 0x3E, 0xEA, 0xA4, 0xD7, 0xCB, 0xB1, 0x5B, 0x0, 0x18, 0x36, 0xFB, 0x56, 0xBD, 0x86, 0xB, 0xD3, 0xC, 0x25, 0xC, 0x15, 0x0, 0x10, 0x36, 0xDD, 0x56, 0xFD, 0xB0, 0xB8, 0x0, 0x22, 0xA4, 0xCE, 0xDB, 0xAA, 0xFA, 0x3C, 0xCF, 0x74, 0xE5, 0x16, 0x0, 0xF6, 0x18, 0x21, 0x14, 0x40, 0x42, 0x20, 0xE2, 0xE7, 0xBB, 0xA4, 0x1, 0x98, 0x4, 0xA4, 0xFC, 0xA0, 0x3, 0x0, 0x18, 0x3C, 0xDD, 0xD6, 0xC2, 0x6, 0x8F, 0xED, 0x97, 0x1A, 0x64, 0x79, 0xA6, 0xDD, 0x32, 0xE8, 0xE8, 0x89, 0xFE, 0x75, 0x73, 0x85, 0x2, 0x0, 0x18, 0x33, 0xFB, 0x16, 0x2, 0xB, 0xF, 0x7, 0x33, 0x5E, 0x2B, 0x74, 0x66, 0xDF, 0x62, 0xE1, 0x41, 0xD0, 0x20, 0xD6, 0x5F, 0x91, 0xCA, 0xEC, 0x5B, 0x9, 0x2D, 0x3C, 0x19, 0xC8, 0x7A, 0x31, 0xCD, 0xC9, 0x5, 0x1, 0x0, 0x18, 0x3C, 0x8, 0xA7, 0x74, 0x10, 0x0, 0x41, 0x3D, 0x0, 0x71, 0x9A, 0x72, 0x8B, 0x81, 0x85, 0x87, 0x7D, 0x43, 0x1F, 0x7, 0x9, 0x0, 0x10, 0x33, 0xEE, 0xD6, 0xA9, 0xBB, 0x80, 0x5, 0x29, 0x25, 0xCA, 0xA5, 0x1E, 0xCC, 0xB5, 0x5B, 0x51, 0xE7, 0xFC, 0xBF, 0xEA, 0x9C, 0x1F, 0xC4, 0x9D, 0x5B, 0x56, 0x9D, 0xFC, 0x40, 0xEA, 0x92, 0xFF, 0x3, 0x0, 0x18, 0x33, 0xED, 0xD6, 0xE5, 0xB9, 0x81, 0x10, 0xAB, 0x23, 0x49, 0x47, 0x8A, 0x9D, 0x1C, 0x0, 0x0, 0x18, 0x33, 0xF5, 0x96, 0xA7, 0xBD, 0xF7, 0x1E, 0xA7, 0x84, 0x25, 0x47, 0xAA, 0xD6, 0x9E, 0x4A, 0xD1, 0x3E, 0x53, 0x0, 0x18, 0x38, 0xF4, 0x56, 0x89, 0xC6, 0x10, 0xFB, 0x30, 0x58, 0x4B, 0x16, 0x0, 0x18, 0x33, 0xF5, 0x96, 0xB3, 0xAF, 0x7F, 0x15, 0x9B, 0x23, 0x88, 0x48, 0xAE, 0xDE, 0x92, 0xAA, 0x6F, 0xFE, 0x0, 0x18, 0x33, 0xE7, 0x56, 0x5, 0xCB, 0x8C, 0x9, 0x32, 0x1E, 0xCE, 0x51, 0xF2, 0x1, 0x10, 0x20, 0xFF, 0xE, 0xE3, 0x29, 0xF, 0xF8, 0xC7, 0xBF, 0x78, 0xD0, 0x24, 0xF2, 0x0, 0x92, 0x2B, 0xF7, 0xFF, 0x5C, 0x66, 0xEE, 0x2D, 0x12, 0x96, 0x8C, 0x4, 0x60, 0x7C, 0x1A, 0x66, 0x24, 0x81, 0x1F, 0x40, 0x0, 0xF, 0x9F, 0x0, 0x0, 0x18, 0x39, 0xEE, 0x16, 0x7F, 0x49, 0xD, 0xF1, 0xA6, 0xDB, 0xCC, 0x15, 0x0, 0x18, 0x26, 0x7, 0x40, 0x25, 0x27, 0x81, 0x61, 0xDD, 0x84, 0x2, 0xB8, 0xE6, 0x33, 0x68, 0xC4, 0x8B, 0x14, 0x0, 0x86, 0xE4, 0xF5, 0x9F, 0x1, 0x0, 0x18, 0x33, 0xC1, 0xD6, 0x3E, 0xC7, 0x10, 0xE5, 0x2, 0xC3, 0xE, 0x31, 0xC6, 0xDD, 0x2A, 0xC9, 0xA0, 0x79, 0x5F, 0x87, 0xB3, 0x61, 0x2, 0x0, 0x19, 0x24, 0xD, 0x80, 0x31, 0x12, 0x62, 0xA7, 0x1C, 0x0, 0x18, 0x38, 0xED, 0xD6, 0x7F, 0x49, 0x4B, 0xC3, 0x3, 0xC3, 0x8B, 0x14, 0x0, 0x18, 0x38, 0xED, 0x96, 0xBD, 0x7, 0x9, 0xDB, 0x6, 0x24, 0xAC, 0x93, 0xC6, 0xDD, 0xEA, 0x28, 0xD9, 0x61, 0x7E, 0x46, 0x4F, 0x99, 0x5E, 0x3A, 0x8, 0x90, 0x4, 0xE0, 0xEE, 0x2E, 0x0, 0x10, 0x38, 0xE7, 0x96, 0xAF, 0x75, 0x3F, 0xD, 0x22, 0xA4, 0x8A, 0xB4, 0xF9, 0x53, 0x75, 0x16, 0x7F, 0x2A, 0xAE, 0x62, 0x70, 0xD5, 0xD0, 0xB, 0x0, 0x0, 0xF4, 0x18, 0x23, 0xF, 0x0, 0x29, 0x99, 0x62, 0xE4, 0x7C, 0xC6, 0xDE, 0xEA, 0x28, 0x19, 0x62, 0x3F, 0x97, 0x77, 0x75, 0x2, 0x0, 0xF8, 0x18, 0x25, 0xF, 0x40, 0x32, 0xA1, 0x5E, 0x45, 0x7D, 0xA6, 0xDC, 0x1A, 0xA9, 0xC9, 0x68, 0x9F, 0xA5, 0xA3, 0x71, 0x2, 0x0, 0x18, 0x36, 0xCC, 0xD6, 0x42, 0xB, 0x55, 0xF2, 0x34, 0xF9, 0x8, 0xD5, 0xE6, 0xDB, 0xA2, 0x60, 0xA9, 0x42, 0xBE, 0x41, 0xEB, 0x78, 0xCB, 0x94, 0x5B, 0xF6, 0x1C, 0x24, 0x4D, 0x33, 0x96, 0x92, 0x63, 0x0, 0xF4, 0x18, 0x23, 0xC, 0x80, 0x15, 0xF8, 0x3F, 0x68, 0x7F, 0xE6, 0xDD, 0xA2, 0x30, 0xD9, 0x31, 0xFF, 0xD5, 0x73, 0x85, 0x2, 0x0, 0x18, 0x26, 0x4, 0x80, 0x1E, 0x87, 0x81, 0x6B, 0x7F, 0xC4, 0x2, 0x98, 0x24, 0x64, 0x58, 0xE9, 0x67, 0xC8, 0x16, 0xC0, 0x13, 0x14, 0x41, 0x52, 0x1, 0x4C, 0x72, 0x21, 0x98, 0xF0, 0x1, 0x0, 0x10, 0x37, 0xE5, 0xD6, 0x30, 0xB9, 0xFF, 0x16, 0xA4, 0x4, 0x63, 0x85, 0x3, 0x0, 0x20, 0x84, 0xFC, 0xF8, 0x3, 0x0, 0x18, 0x32, 0xCF, 0x16, 0xC3, 0xC8, 0x4E, 0xDE, 0xAC, 0x97, 0x8A, 0x74, 0xC6, 0xDA, 0xB2, 0xBE, 0x18, 0xE1, 0x97, 0x70, 0x74, 0x85, 0x52, 0x1E, 0x38, 0xE, 0x1D, 0x30, 0xF1, 0x5, 0x0, 0x19, 0x21, 0xF, 0xC0, 0x29, 0x94, 0xE0, 0x64, 0x1C, 0x0, 0x1D, 0xF2, 0x18, 0x21, 0xF, 0x80, 0x35, 0x89, 0xC0, 0xCA, 0x5B, 0xB6, 0xC0, 0xDD, 0x78, 0x7A, 0x0, 0xF7, 0x18, 0x21, 0x1D, 0xC0, 0x31, 0xB1, 0xE1, 0x46, 0x3C, 0xE4, 0x0, 0xA0, 0x65, 0x43, 0x10, 0xE5, 0xA7, 0x54, 0x0, 0xB5, 0x88, 0x86, 0x8D, 0x73, 0x0, 0x18, 0x36, 0xF4, 0x56, 0x89, 0x51, 0xD7, 0x2, 0xAA, 0xBB, 0xE9, 0x34, 0xC5, 0x2, 0xD8, 0x8, 0xB0, 0xC3, 0x84, 0xD0, 0xD4, 0x5B, 0x25, 0x45, 0x4C, 0x8, 0xC0, 0xEE, 0x9B, 0x5C, 0x0, 0x18, 0x35, 0xEF, 0x16, 0x37, 0x2F, 0xFF, 0x6, 0x9E, 0x45, 0xAB, 0x6A, 0x8A, 0xF5, 0x76, 0x5B, 0x9D, 0xD6, 0x6F, 0xAC, 0xD2, 0x5B, 0xD, 0x0, 0x18, 0x33, 0xDE, 0x96, 0xA7, 0x33, 0x83, 0x19, 0x22, 0xA5, 0xC4, 0x65, 0xAA, 0xA5, 0x39, 0x4C, 0xB1, 0x14, 0x3, 0x0, 0x18, 0x35, 0xCD, 0x96, 0x3E, 0xC7, 0xCA, 0xC1, 0x7C, 0x42, 0xB, 0xB4, 0xE6, 0xD9, 0x5A, 0x30, 0xE1, 0x89, 0x1F, 0xC5, 0x7E, 0x79, 0xDA, 0x54, 0x5B, 0x5, 0x1E, 0x25, 0xAE, 0x6, 0xA, 0xB9, 0x49, 0x0, 0x18, 0x33, 0xED, 0x96, 0xA9, 0xBB, 0xBF, 0x0, 0xA9, 0x23, 0x4B, 0x48, 0xAE, 0xDD, 0x92, 0x3A, 0x69, 0xF7, 0x52, 0x2D, 0xE5, 0x5E, 0xCA, 0xAD, 0xDC, 0x4B, 0xB5, 0x75, 0xF7, 0x39, 0x76, 0x2B, 0xEE, 0xDC, 0x9A, 0xFA, 0xAA, 0xE6, 0x51, 0x1C, 0xD5, 0x5C, 0xCA, 0xAB, 0xBA, 0xC7, 0x5D, 0x5B, 0x53, 0x55, 0xDD, 0x1D, 0x0, 0x18, 0x26, 0x3, 0x0, 0x21, 0x8E, 0x1F, 0x45, 0x7A, 0x65, 0x0, 0xB8, 0x84, 0x11, 0x54, 0xD1, 0xCA, 0x78, 0x5B, 0xFC, 0x37, 0x22, 0x17, 0xFB, 0x89, 0xAE, 0x51, 0x99, 0x72, 0xCB, 0xBF, 0xA5, 0xA5, 0xD9, 0xC1, 0x81, 0x35, 0xA, 0x0, 0x18, 0x33, 0xE7, 0x56, 0x5, 0xCB, 0x8C, 0x9, 0x32, 0x1E, 0xCE, 0x51, 0xF2, 0x1, 0x10, 0x20, 0xFF, 0xE, 0xE3, 0x29, 0xF, 0xF8, 0xC7, 0xBF, 0x78, 0xD0, 0x24, 0xF2, 0x0, 0x92, 0x2B, 0xF7, 0xFF, 0x5C, 0x0, 0x0, 0xF5, 0x18, 0x25, 0x5, 0x0, 0x2A, 0x27, 0x21, 0x83, 0xBC, 0xA5, 0x2, 0xD0, 0x66, 0x46, 0x24, 0xD9, 0x3, 0x0, 0x18, 0x31, 0xED, 0x16, 0x7, 0x89, 0xC, 0xE7, 0xB4, 0xF9, 0xAB, 0x54, 0x12, 0x0, 0x0, 0xFC, 0x7, 0xE, 0x0, 0x62, 0xDA, 0x2D, 0x73, 0xE, 0x12, 0xA6, 0x13, 0xC9, 0x16, 0x6A, 0xE4, 0x3, 0x20, 0x40, 0x0, 0xFC, 0x68, 0x47, 0x2E, 0x0, 0x2, 0x20, 0x0, 0xE0, 0x85, 0x64, 0x1, 0xE0, 0x0, 0xE, 0x14, 0xA8, 0x47, 0x26, 0x0, 0x2, 0x20, 0xC1, 0x5E, 0xBC, 0xCC, 0xB7, 0xE5, 0x21, 0x12, 0xC2, 0x38, 0xAD, 0x76, 0x3, 0x8D, 0x2C, 0x0, 0xE0, 0x11, 0x22, 0xA0, 0xE, 0x0, 0x18, 0x32, 0xED, 0x16, 0x7, 0x89, 0xC, 0xE7, 0xB4, 0xF9, 0xAB, 0x54, 0x12, 0x0, 0x0, 0xFC, 0x7, 0xE, 0x0, 0x64, 0xD8, 0x2D, 0x73, 0xE, 0x16, 0xA6, 0x11, 0x49, 0x56, 0xAA, 0x24, 0x0, 0x0, 0xC0, 0x1, 0x1, 0xE2, 0x4B, 0x2E, 0x0, 0x2, 0x4, 0xC0, 0x8F, 0xB6, 0x24, 0x0, 0x20, 0x0, 0x2, 0x0, 0x5E, 0x48, 0x2, 0x0, 0xE, 0xE0, 0x40, 0x81, 0xFA, 0xA4, 0x3, 0x20, 0x0, 0x12, 0xEC, 0xC5, 0xCF, 0x78, 0x5B, 0x1E, 0x22, 0x21, 0x8C, 0xD3, 0x6A, 0x37, 0xD0, 0xC9, 0x0, 0x0, 0x1E, 0x21, 0x2, 0xEA, 0x0, 0x0, 0x10, 0x33, 0xED, 0x96, 0xAB, 0xB1, 0x3F, 0x43, 0xB0, 0x64, 0x8A, 0xAD, 0x18, 0xC4, 0x9D, 0x5B, 0x55, 0x1E, 0xBD, 0x20, 0xCE, 0xDB, 0xB2, 0xBB, 0xB6, 0x0, 0x0, 0x19, 0x31, 0xDD, 0xD6, 0xC2, 0x6, 0x8F, 0xED, 0x97, 0x1A, 0x64, 0x79, 0xA6, 0xDD, 0x32, 0xE8, 0xE8, 0x89, 0xFE, 0x75, 0x73, 0x85, 0x2, 0x0, 0x18, 0x2A, 0x17, 0x0, 0x4A, 0x1C, 0x24, 0x65, 0x1C, 0x0, 0x18, 0x34, 0xDD, 0xD6, 0x80, 0x6, 0xF, 0xFF, 0x15, 0x9C, 0x2A, 0x54, 0x32, 0x0, 0x92, 0xB4, 0x81, 0x14, 0x1E, 0x24, 0xB, 0x0, 0x1, 0x0, 0x20, 0x5F, 0x79, 0x66, 0xDF, 0xA2, 0xE0, 0x98, 0x69, 0x3F, 0x76, 0x43, 0xA1, 0x46, 0x2, 0x30, 0x80, 0x0, 0x60, 0x5E, 0xC8, 0x24, 0x80, 0xA9, 0x0, 0x2, 0xFE, 0x3, 0x0, 0x0, 0x18, 0x2E, 0x3, 0x80, 0x21, 0xF, 0x0, 0x29, 0x98, 0xC6, 0xDB, 0x22, 0x30, 0xE9, 0x19, 0xBD, 0x80, 0x27, 0x9, 0x3, 0x0, 0x18, 0x32, 0xD6, 0x96, 0xA9, 0xAB, 0x3F, 0x11, 0x30, 0xE4, 0xEA, 0x26, 0x8A, 0xA2, 0xD5, 0xCF, 0x55, 0x5B, 0x53, 0x1F, 0xB5, 0xBE, 0xAA, 0xA4, 0xB8, 0x53, 0x9D, 0x35, 0x73, 0xAA, 0xB3, 0x6A, 0x1E, 0xF5, 0x51, 0xCD, 0xE7, 0xAC, 0x2D, 0x0, 0x18, 0x32, 0xE4, 0x96, 0x7, 0xB, 0x44, 0xF2, 0xB2, 0x37, 0x8B, 0x55, 0x32, 0x0, 0x8E, 0x27, 0x80, 0x87, 0x3E, 0x24, 0x3, 0x1C, 0x7, 0x82, 0xF7, 0x0, 0x41, 0xA6, 0xDC, 0xAA, 0xB7, 0x28, 0x61, 0x38, 0x40, 0x44, 0xD9, 0x4A, 0x2, 0xC8, 0x11, 0x4, 0xF1, 0x9D, 0xC5, 0x4C, 0xB9, 0xE5, 0x1, 0x31, 0xB2, 0x7A, 0x4C, 0xA6, 0xAA, 0x9D, 0xF9, 0xB6, 0x0, 0x3C, 0x6A, 0x38, 0x67, 0xCD, 0x56, 0xB5, 0x93, 0xF, 0x0, 0xE0, 0x7, 0x48, 0x8C, 0x3E, 0xC3, 0x6D, 0xB9, 0x5B, 0x88, 0xF0, 0xEC, 0xF, 0xAA, 0x64, 0x1, 0x0, 0x18, 0x33, 0xEF, 0xD6, 0x65, 0xFD, 0xC0, 0x4, 0x27, 0x25, 0x28, 0x47, 0x8A, 0x96, 0x1F, 0xCC, 0xD5, 0x5B, 0x52, 0xAD, 0xF4, 0x60, 0xAA, 0x9D, 0xFD, 0x53, 0x99, 0x94, 0xFB, 0x2A, 0x93, 0x6E, 0x5F, 0x57, 0x6E, 0x1, 0x0, 0xF4, 0x18, 0x24, 0x7, 0xC0, 0x21, 0x20, 0x9C, 0xC3, 0x5C, 0xC6, 0xDC, 0xE2, 0xF7, 0xE8, 0x19, 0xDF, 0x65, 0xC3, 0x85, 0x2, 0x0, 0x18, 0x35, 0xEE, 0x16, 0x47, 0x8B, 0x48, 0xFF, 0xB0, 0x98, 0x6C, 0x74, 0xC6, 0xDC, 0x8A, 0x70, 0xE1, 0xE1, 0x3F, 0x6, 0x83, 0x8D, 0xDE, 0x90, 0x5B, 0x2E, 0x19, 0x33, 0xC, 0xC0, 0x60, 0x34, 0x50, 0x0, 0xF4, 0x18, 0x21, 0x6, 0x80, 0x21, 0x92, 0x7F, 0xC8, 0x5B, 0xA6, 0xDE, 0x2A, 0xA1, 0xC9, 0x68, 0xFF, 0x86, 0xB3, 0x65, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

struct lpc12_t
{
	int     rpt, cnt;       /* Repeat counter, Period down-counter.         */
	UINT32  per, rng;       /* Period, Amplitude, Random Number Generator   */
	int     amp;
	INT16   f_coef[6];      /* F0 through F5.                               */
	INT16   b_coef[6];      /* B0 through B5.                               */
	INT16   z_data[6][2];   /* Time-delay data for the filter stages.       */
	UINT8   r[16];          /* The encoded register set.                    */
	int     interp;
};

INT16 m_scratch[4096];

int            m_sby_line;        /* Standby line state                           */
int            m_cur_len;         /* Fullness of current sound buffer.            */

int            m_silent;          /* Flag: SP0256 is silent.                      */


struct lpc12_t m_filt;            /* 12-pole filter                               */
int            m_lrq;             /* Load ReQuest.  == 0 if we can accept a load  */
int            m_ald;             /* Address LoaD.  < 0 if no command pending.    */
int            m_pc;              /* Microcontroller's PC value.                  */
int            m_stack;           /* Microcontroller's PC stack.                  */
int            m_fifo_sel;        /* True when executing from FIFO.               */
int            m_halted;          /* True when CPU is halted.                     */
UINT32         m_mode;            /* Mode register.                               */
UINT32         m_page;            /* Page set by SETPAGE                          */

UINT32         m_sc_head;         /* Head pointer into scratch circular buf       */
UINT32         m_sc_tail;         /* Tail pointer into scratch circular buf       */

int            m_sby_line;        /* Standby line state                           */
int            m_cur_len;         /* Fullness of current sound buffer.            */

int            m_silent;          /* Flag: SP0256 is silent.                      */
UINT32         m_fifo_head;       /* FIFO head pointer (where new data goes).     */
UINT32         m_fifo_tail;       /* FIFO tail pointer (where data comes from).   */
UINT32         m_fifo_bitp;       /* FIFO bit-pointer (for partial decles).       */
UINT16         m_fifo[64];        /* The 64-decle FIFO.                           */


#define CLOCK_DIVIDER (7*6*8)
//#define HIGH_QUALITY

#define SCBUF_SIZE   (4096)             /* Must be power of 2               */
#define SCBUF_MASK   (SCBUF_SIZE - 1)
#define PER_PAUSE    (64)               /* Equiv timing period for pauses.  */
#define PER_NOISE    (64)               /* Equiv timing period for noise.   */

#define FIFO_ADDR    (0x1800 << 3)      /* SP0256 address of SPB260 speech FIFO. = 49152 decimal  */

#define SET_SBY(line_state) {                  \
	if (m_sby_line != line_state)           \
	{                                          \
		m_sby_line = line_state;             \
		m_sby_cb(m_sby_line);  \
	}                                          \
}

/* ======================================================================== */
/*  qtbl  -- Coefficient Quantization Table.  This comes from a             */
/*              SP0250 data sheet, and should be correct for SP0256.        */
/* ======================================================================== */
static const INT16 qtbl[128] =
{
	0,      9,      17,     25,     33,     41,     49,     57,
	65,     73,     81,     89,     97,     105,    113,    121,
	129,    137,    145,    153,    161,    169,    177,    185,
	193,    201,    209,    217,    225,    233,    241,    249,
	257,    265,    273,    281,    289,    297,    301,    305,
	309,    313,    317,    321,    325,    329,    333,    337,
	341,    345,    349,    353,    357,    361,    365,    369,
	373,    377,    381,    385,    389,    393,    397,    401,
	405,    409,    413,    417,    421,    425,    427,    429,
	431,    433,    435,    437,    439,    441,    443,    445,
	447,    449,    451,    453,    455,    457,    459,    461,
	463,    465,    467,    469,    471,    473,    475,    477,
	479,    481,    482,    483,    484,    485,    486,    487,
	488,    489,    490,    491,    492,    493,    494,    495,
	496,    497,    498,    499,    500,    501,    502,    503,
	504,    505,    506,    507,    508,    509,    510,    511
};

void sp0256_init()
{

	/* -------------------------------------------------------------------- */
	/*  Configure our internal variables.                                   */
	/* -------------------------------------------------------------------- */
	m_filt.rng = 1;

	memset(m_rom,0,65536);
	memcpy(m_rom+0x1000,m_roma,2048);

	/* -------------------------------------------------------------------- */
	/*  Set up the microsequencer's initial state.                          */
	/* -------------------------------------------------------------------- */
	m_halted   = 1; // was 1
	m_filt.rpt = -1;
	m_lrq      = 0x8000;
	m_page     = 0x1000 << 3; //32768
	m_silent   = 1;

	/* -------------------------------------------------------------------- */
	/*  Setup the ROM.                                                      */
	/* -------------------------------------------------------------------- */
	// the rom is not supposed to be reversed first; according to Joe Zbiciak.
	// see http://forums.bannister.org/ubbthreads.php?ubb=showflat&Number=72385#Post72385
	// TODO: because of this, check if the bitrev functions are even used anywhere else
	//	bitrevbuff(m_rom, 0, 0xffff);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void reset()
{
	// reset FIFO and SP0256
	m_fifo_head = m_fifo_tail = m_fifo_bitp = 0;

	memset(&m_filt, 0, sizeof(m_filt));
	m_halted   = 1;
	m_filt.rpt = -1;
	m_filt.rng = 1;
	m_lrq      = 0x8000;
	m_ald      = 0x0000;
	m_pc       = 0x0000;
	m_stack    = 0x0000;
	m_fifo_sel = 0;
	m_mode     = 0;
	m_page     = 0x1000 << 3;
	m_silent   = 1;
	m_sby_line = 0;
	//	m_drq_cb(1);
	//	SET_SBY(1)

	  m_lrq = 0;
//	m_lrq_timer->adjust(attotime::from_ticks(50, m_clock));
}



/* ======================================================================== */
/*  LIMIT            -- Limiter function for digital sample output.         */
/* ======================================================================== */
static inline INT16 limit(INT16 s)
{
#ifdef HIGH_QUALITY /* Higher quality than the original, but who cares? */
	if (s >  8191) return  8191;
	if (s < -8192) return -8192;
#else
	if (s >  127) return  127;
	if (s < -128) return -128;
#endif
	return s;
}

/* ======================================================================== */
/*  LPC12_UPDATE     -- Update the 12-pole filter, outputting samples.      */
/* ======================================================================== */
static inline int lpc12_update(struct lpc12_t *f, int num_samp, INT16 *out, UINT32 *optr)
{
	int i, j;
	INT16 samp;
	int do_int;
	int oidx = *optr;

	/* -------------------------------------------------------------------- */
	/*  Iterate up to the desired number of samples.  We actually may       */
	/*  break out early if our repeat count expires.                        */
	/* -------------------------------------------------------------------- */
	for (i = 0; i < num_samp; i++)
	{
		/* ---------------------------------------------------------------- */
		/*  Generate a series of periodic impulses, or random noise.        */
		/* ---------------------------------------------------------------- */
		do_int = 0;
		samp   = 0;
		if (f->per)
		{
			if (f->cnt <= 0)
			{
				f->cnt += f->per;
				samp    = f->amp;
				f->rpt--;
				do_int  = f->interp;

				for (j = 0; j < 6; j++)
					f->z_data[j][1] = f->z_data[j][0] = 0;

			} else
			{
				samp = 0;
				f->cnt--;
			}

		} else
		{
			int bit;

			if (--f->cnt <= 0)
			{
				do_int = f->interp;
				f->cnt = PER_NOISE;
				f->rpt--;
				for (j = 0; j < 6; j++)
					f->z_data[j][0] = f->z_data[j][1] = 0;
			}

			bit = f->rng & 1;
			f->rng = (f->rng >> 1) ^ (bit ? 0x4001 : 0);

			if (bit) { samp =  f->amp; }
			else     { samp = -f->amp; }
		}

		/* ---------------------------------------------------------------- */
		/*  If we need to, process the interpolation registers.             */
		/* ---------------------------------------------------------------- */
		if (do_int)
		{
			f->r[0] += f->r[14];
			f->r[1] += f->r[15];

			f->amp   = (f->r[0] & 0x1F) << (((f->r[0] & 0xE0) >> 5) + 0);
			f->per   = f->r[1];

			do_int   = 0;
		}

		/* ---------------------------------------------------------------- */
		/*  Stop if we expire our repeat counter and return the actual      */
		/*  number of samples we did.                                       */
		/* ---------------------------------------------------------------- */
		if (f->rpt <= 0) break;

		/* ---------------------------------------------------------------- */
		/*  Each 2nd order stage looks like one of these.  The App. Manual  */
		/*  gives the first form, the patent gives the second form.         */
		/*  They're equivalent except for time delay.  I implement the      */
		/*  first form.   (Note: 1/Z == 1 unit of time delay.)              */
		/*                                                                  */
		/*          ---->(+)-------->(+)----------+------->                 */
		/*                ^           ^           |                         */
		/*                |           |           |                         */
		/*                |           |           |                         */
		/*               [B]        [2*F]         |                         */
		/*                ^           ^           |                         */
		/*                |           |           |                         */
		/*                |           |           |                         */
		/*                +---[1/Z]<--+---[1/Z]<--+                         */
		/*                                                                  */
		/*                                                                  */
		/*                +---[2*F]<---+                                    */
		/*                |            |                                    */
		/*                |            |                                    */
		/*                v            |                                    */
		/*          ---->(+)-->[1/Z]-->+-->[1/Z]---+------>                 */
		/*                ^                        |                        */
		/*                |                        |                        */
		/*                |                        |                        */
		/*                +-----------[B]<---------+                        */
		/*                                                                  */
		/* ---------------------------------------------------------------- */
		for (j = 0; j < 6; j++)
		{
			samp += (((int)f->b_coef[j] * (int)f->z_data[j][1]) >> 9);
			samp += (((int)f->f_coef[j] * (int)f->z_data[j][0]) >> 8);

			f->z_data[j][1] = f->z_data[j][0];
			f->z_data[j][0] = samp;
		}
		//		printf("SAMP %d\n",samp);
		printf("%c",limit(samp>>2)+128);

#ifdef HIGH_QUALITY /* Higher quality than the original, but who cares? */
		//		out[oidx++ & SCBUF_MASK] = limit(samp) << 2;
		
#else
		//		out[oidx++ & SCBUF_MASK] = (limit(samp >> 4) << 8);
#endif
	}

	*optr = oidx;

	return i;
}

static const int stage_map[6] = { 0, 1, 2, 3, 4, 5 };

/* ======================================================================== */
/*  LPC12_REGDEC -- Decode the register set in the filter bank.             */
/* ======================================================================== */
static inline void lpc12_regdec(struct lpc12_t *f)
{
	int i;

	/* -------------------------------------------------------------------- */
	/*  Decode the Amplitude and Period registers.  Force the 'cnt' to 0    */
	/*  to get an initial impulse.  We compensate elsewhere by setting      */
	/*  the repeat count to "repeat + 1".                                   */
	/* -------------------------------------------------------------------- */
	f->amp = (f->r[0] & 0x1F) << (((f->r[0] & 0xE0) >> 5) + 0);
	f->cnt = 0;
	f->per = f->r[1];

	/* -------------------------------------------------------------------- */
	/*  Decode the filter coefficients from the quant table.                */
	/* -------------------------------------------------------------------- */
	for (i = 0; i < 6; i++)
	{
		#define IQ(x) (((x) & 0x80) ? qtbl[0x7F & -(x)] : -qtbl[(x)])

		f->b_coef[stage_map[i]] = IQ(f->r[2 + 2*i]);
		f->f_coef[stage_map[i]] = IQ(f->r[3 + 2*i]);
	}

	/* -------------------------------------------------------------------- */
	/*  Set the Interp flag based on whether we have interpolation parms    */
	/* -------------------------------------------------------------------- */
	f->interp = f->r[14] || f->r[15];

	return;
}

/* ======================================================================== */
/*  SP0256_DATAFMT   -- Data format table for the SP0256's microsequencer   */
/*                                                                          */
/*  len     4 bits      Length of field to extract                          */
/*  lshift  4 bits      Left-shift amount on field                          */
/*  param   4 bits      Parameter number being updated                      */
/*  delta   1 bit       This is a delta-update.  (Implies sign-extend)      */
/*  field   1 bit       This is a field replace.                            */
/*  clr5    1 bit       Clear F5, B5.                                       */
/*  clrall  1 bit       Clear all before doing this update                  */
/* ======================================================================== */

#define CR(l,s,p,d,f,c5,ca)         \
		(                           \
			(((l)  & 15) <<  0) |   \
			(((s)  & 15) <<  4) |   \
			(((p)  & 15) <<  8) |   \
			(((d)  &  1) << 12) |   \
			(((f)  &  1) << 13) |   \
			(((c5) &  1) << 14) |   \
			(((ca) &  1) << 15)     \
		)

#define CR_DELTA  CR(0,0,0,1,0,0,0)
#define CR_FIELD  CR(0,0,0,0,1,0,0)
#define CR_CLR5   CR(0,0,0,0,0,1,0)
#define CR_CLRA   CR(0,0,0,0,0,0,1)
#define CR_LEN(x) ((x) & 15)
#define CR_SHF(x) (((x) >> 4) & 15)
#define CR_PRM(x) (((x) >> 8) & 15)

enum { AM = 0, PR, B0, F0, B1, F1, B2, F2, B3, F3, B4, F4, B5, F5, IA, IP };

static const UINT16 sp0256_datafmt[] =
{
	/* -------------------------------------------------------------------- */
	/*  OPCODE 1111: PAUSE                                                  */
	/* -------------------------------------------------------------------- */
	/*    0 */  CR( 0,  0,  0,  0,  0,  0,  1),     /*  Clear all   */

	/* -------------------------------------------------------------------- */
	/*  Opcode 0001: LOADALL                                                */
	/* -------------------------------------------------------------------- */
				/* All modes                */
	/*    1 */  CR( 8,  0,  AM, 0,  0,  0,  1),     /*  Amplitude   */
	/*    2 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*    3 */  CR( 8,  0,  B0, 0,  0,  0,  0),     /*  B0          */
	/*    4 */  CR( 8,  0,  F0, 0,  0,  0,  0),     /*  F0          */
	/*    5 */  CR( 8,  0,  B1, 0,  0,  0,  0),     /*  B1          */
	/*    6 */  CR( 8,  0,  F1, 0,  0,  0,  0),     /*  F1          */
	/*    7 */  CR( 8,  0,  B2, 0,  0,  0,  0),     /*  B2          */
	/*    8 */  CR( 8,  0,  F2, 0,  0,  0,  0),     /*  F2          */
	/*    9 */  CR( 8,  0,  B3, 0,  0,  0,  0),     /*  B3          */
	/*   10 */  CR( 8,  0,  F3, 0,  0,  0,  0),     /*  F3          */
	/*   11 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
	/*   12 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
	/*   13 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
	/*   14 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
				/* Mode 01 and 11 only      */
	/*   15 */  CR( 8,  0,  IA, 0,  0,  0,  0),     /*  Amp Interp  */
	/*   16 */  CR( 8,  0,  IP, 0,  0,  0,  0),     /*  Pit Interp  */

	/* -------------------------------------------------------------------- */
	/*  Opcode 0100: LOAD_4                                                 */
	/* -------------------------------------------------------------------- */
				/* Mode 00 and 01           */
	/*   17 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
	/*   18 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*   19 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
	/*   20 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
	/*   21 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
	/*   22 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
				/* Mode 01 only             */
	/*   23 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
	/*   24 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */

				/* Mode 10 and 11           */
	/*   25 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
	/*   26 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*   27 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
	/*   28 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
	/*   29 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
	/*   30 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
				/* Mode 11 only             */
	/*   31 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
	/*   32 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */

	/* -------------------------------------------------------------------- */
	/*  Opcode 0110: SETMSB_6                                               */
	/* -------------------------------------------------------------------- */
				/* Mode 00 only             */
	/*   33 */  CR( 0,  0,  0,  0,  0,  1,  0),     /*  Clear 5     */
				/* Mode 00 and 01           */
	/*   34 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
	/*   35 */  CR( 6,  2,  F3, 0,  1,  0,  0),     /*  F3 (5 MSBs) */
	/*   36 */  CR( 6,  2,  F4, 0,  1,  0,  0),     /*  F4 (5 MSBs) */
				/* Mode 01 only             */
	/*   37 */  CR( 8,  0,  F5, 0,  1,  0,  0),     /*  F5 (5 MSBs) */

				/* Mode 10 only             */
	/*   38 */  CR( 0,  0,  0,  0,  0,  1,  0),     /*  Clear 5     */
				/* Mode 10 and 11           */
	/*   39 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
	/*   40 */  CR( 7,  1,  F3, 0,  1,  0,  0),     /*  F3 (6 MSBs) */
	/*   41 */  CR( 8,  0,  F4, 0,  1,  0,  0),     /*  F4 (6 MSBs) */
				/* Mode 11 only             */
	/*   42 */  CR( 8,  0,  F5, 0,  1,  0,  0),     /*  F5 (6 MSBs) */

	/*   43 */  0,  /* unused */
	/*   44 */  0,  /* unused */

	/* -------------------------------------------------------------------- */
	/*  Opcode 1001: DELTA_9                                                */
	/* -------------------------------------------------------------------- */
				/* Mode 00 and 01           */
	/*   45 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
	/*   46 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
	/*   47 */  CR( 3,  4,  B0, 1,  0,  0,  0),     /*  B0 4 MSBs   */
	/*   48 */  CR( 3,  3,  F0, 1,  0,  0,  0),     /*  F0 5 MSBs   */
	/*   49 */  CR( 3,  4,  B1, 1,  0,  0,  0),     /*  B1 4 MSBs   */
	/*   50 */  CR( 3,  3,  F1, 1,  0,  0,  0),     /*  F1 5 MSBs   */
	/*   51 */  CR( 3,  4,  B2, 1,  0,  0,  0),     /*  B2 4 MSBs   */
	/*   52 */  CR( 3,  3,  F2, 1,  0,  0,  0),     /*  F2 5 MSBs   */
	/*   53 */  CR( 3,  3,  B3, 1,  0,  0,  0),     /*  B3 5 MSBs   */
	/*   54 */  CR( 4,  2,  F3, 1,  0,  0,  0),     /*  F3 6 MSBs   */
	/*   55 */  CR( 4,  1,  B4, 1,  0,  0,  0),     /*  B4 7 MSBs   */
	/*   56 */  CR( 4,  2,  F4, 1,  0,  0,  0),     /*  F4 6 MSBs   */
				/* Mode 01 only             */
	/*   57 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
	/*   58 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

				/* Mode 10 and 11           */
	/*   59 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
	/*   60 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
	/*   61 */  CR( 4,  1,  B0, 1,  0,  0,  0),     /*  B0 7 MSBs   */
	/*   62 */  CR( 4,  2,  F0, 1,  0,  0,  0),     /*  F0 6 MSBs   */
	/*   63 */  CR( 4,  1,  B1, 1,  0,  0,  0),     /*  B1 7 MSBs   */
	/*   64 */  CR( 4,  2,  F1, 1,  0,  0,  0),     /*  F1 6 MSBs   */
	/*   65 */  CR( 4,  1,  B2, 1,  0,  0,  0),     /*  B2 7 MSBs   */
	/*   66 */  CR( 4,  2,  F2, 1,  0,  0,  0),     /*  F2 6 MSBs   */
	/*   67 */  CR( 4,  1,  B3, 1,  0,  0,  0),     /*  B3 7 MSBs   */
	/*   68 */  CR( 5,  1,  F3, 1,  0,  0,  0),     /*  F3 7 MSBs   */
	/*   69 */  CR( 5,  0,  B4, 1,  0,  0,  0),     /*  B4 8 MSBs   */
	/*   70 */  CR( 5,  0,  F4, 1,  0,  0,  0),     /*  F4 8 MSBs   */
				/* Mode 11 only             */
	/*   71 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
	/*   72 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

	/* -------------------------------------------------------------------- */
	/*  Opcode 1010: SETMSB_A                                               */
	/* -------------------------------------------------------------------- */
				/* Mode 00 only             */
	/*   73 */  CR( 0,  0,  0,  0,  0,  1,  0),     /*  Clear 5     */
				/* Mode 00 and 01           */
	/*   74 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
	/*   75 */  CR( 5,  3,  F0, 0,  1,  0,  0),     /*  F0 (5 MSBs) */
	/*   76 */  CR( 5,  3,  F1, 0,  1,  0,  0),     /*  F1 (5 MSBs) */
	/*   77 */  CR( 5,  3,  F2, 0,  1,  0,  0),     /*  F2 (5 MSBs) */

				/* Mode 10 only             */
	/*   78 */  CR( 0,  0,  0,  0,  0,  1,  0),     /*  Clear 5     */
				/* Mode 10 and 11           */
	/*   79 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
	/*   80 */  CR( 6,  2,  F0, 0,  1,  0,  0),     /*  F0 (6 MSBs) */
	/*   81 */  CR( 6,  2,  F1, 0,  1,  0,  0),     /*  F1 (6 MSBs) */
	/*   82 */  CR( 6,  2,  F2, 0,  1,  0,  0),     /*  F2 (6 MSBs) */

	/* -------------------------------------------------------------------- */
	/*  Opcode 0010: LOAD_2  Mode 00 and 10                                 */
	/*  Opcode 1100: LOAD_C  Mode 00 and 10                                 */
	/* -------------------------------------------------------------------- */
				/* LOAD_2, LOAD_C  Mode 00  */
	/*   83 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
	/*   84 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*   85 */  CR( 3,  4,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
	/*   86 */  CR( 5,  3,  F0, 0,  0,  0,  0),     /*  F0          */
	/*   87 */  CR( 3,  4,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
	/*   88 */  CR( 5,  3,  F1, 0,  0,  0,  0),     /*  F1          */
	/*   89 */  CR( 3,  4,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
	/*   90 */  CR( 5,  3,  F2, 0,  0,  0,  0),     /*  F2          */
	/*   91 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
	/*   92 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
	/*   93 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
	/*   94 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
				/* LOAD_2 only              */
	/*   95 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
	/*   96 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

				/* LOAD_2, LOAD_C  Mode 10  */
	/*   97 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
	/*   98 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*   99 */  CR( 6,  1,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
	/*  100 */  CR( 6,  2,  F0, 0,  0,  0,  0),     /*  F0          */
	/*  101 */  CR( 6,  1,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
	/*  102 */  CR( 6,  2,  F1, 0,  0,  0,  0),     /*  F1          */
	/*  103 */  CR( 6,  1,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
	/*  104 */  CR( 6,  2,  F2, 0,  0,  0,  0),     /*  F2          */
	/*  105 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
	/*  106 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
	/*  107 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
	/*  108 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
				/* LOAD_2 only              */
	/*  109 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
	/*  110 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

	/* -------------------------------------------------------------------- */
	/*  OPCODE 1101: DELTA_D                                                */
	/* -------------------------------------------------------------------- */
				/* Mode 00 and 01           */
	/*  111 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
	/*  112 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
	/*  113 */  CR( 3,  3,  B3, 1,  0,  0,  0),     /*  B3 5 MSBs   */
	/*  114 */  CR( 4,  2,  F3, 1,  0,  0,  0),     /*  F3 6 MSBs   */
	/*  115 */  CR( 4,  1,  B4, 1,  0,  0,  0),     /*  B4 7 MSBs   */
	/*  116 */  CR( 4,  2,  F4, 1,  0,  0,  0),     /*  F4 6 MSBs   */
				/* Mode 01 only             */
	/*  117 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
	/*  118 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

				/* Mode 10 and 11           */
	/*  119 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
	/*  120 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
	/*  121 */  CR( 4,  1,  B3, 1,  0,  0,  0),     /*  B3 7 MSBs   */
	/*  122 */  CR( 5,  1,  F3, 1,  0,  0,  0),     /*  F3 7 MSBs   */
	/*  123 */  CR( 5,  0,  B4, 1,  0,  0,  0),     /*  B4 8 MSBs   */
	/*  124 */  CR( 5,  0,  F4, 1,  0,  0,  0),     /*  F4 8 MSBs   */
				/* Mode 11 only             */
	/*  125 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
	/*  126 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

	/* -------------------------------------------------------------------- */
	/*  OPCODE 1110: LOAD_E                                                 */
	/* -------------------------------------------------------------------- */
	/*  127 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
	/*  128 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */

	/* -------------------------------------------------------------------- */
	/*  Opcode 0010: LOAD_2  Mode 01 and 11                                 */
	/*  Opcode 1100: LOAD_C  Mode 01 and 11                                 */
	/* -------------------------------------------------------------------- */
				/* LOAD_2, LOAD_C  Mode 01  */
	/*  129 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
	/*  130 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*  131 */  CR( 3,  4,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
	/*  132 */  CR( 5,  3,  F0, 0,  0,  0,  0),     /*  F0          */
	/*  133 */  CR( 3,  4,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
	/*  134 */  CR( 5,  3,  F1, 0,  0,  0,  0),     /*  F1          */
	/*  135 */  CR( 3,  4,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
	/*  136 */  CR( 5,  3,  F2, 0,  0,  0,  0),     /*  F2          */
	/*  137 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
	/*  138 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
	/*  139 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
	/*  140 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
	/*  141 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
	/*  142 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
				/* LOAD_2 only              */
	/*  143 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
	/*  144 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

				/* LOAD_2, LOAD_C  Mode 11  */
	/*  145 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
	/*  146 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*  147 */  CR( 6,  1,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
	/*  148 */  CR( 6,  2,  F0, 0,  0,  0,  0),     /*  F0          */
	/*  149 */  CR( 6,  1,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
	/*  150 */  CR( 6,  2,  F1, 0,  0,  0,  0),     /*  F1          */
	/*  151 */  CR( 6,  1,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
	/*  152 */  CR( 6,  2,  F2, 0,  0,  0,  0),     /*  F2          */
	/*  153 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
	/*  154 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
	/*  155 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
	/*  156 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
	/*  157 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
	/*  158 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
				/* LOAD_2 only              */
	/*  159 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
	/*  160 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

	/* -------------------------------------------------------------------- */
	/*  Opcode 0011: SETMSB_3                                               */
	/*  Opcode 0101: SETMSB_5                                               */
	/* -------------------------------------------------------------------- */
				/* Mode 00 only             */
	/*  161 */  CR( 0,  0,  0,  0,  0,  1,  0),     /*  Clear 5     */
				/* Mode 00 and 01           */
	/*  162 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
	/*  163 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*  164 */  CR( 5,  3,  F0, 0,  1,  0,  0),     /*  F0 (5 MSBs) */
	/*  165 */  CR( 5,  3,  F1, 0,  1,  0,  0),     /*  F1 (5 MSBs) */
	/*  166 */  CR( 5,  3,  F2, 0,  1,  0,  0),     /*  F2 (5 MSBs) */
				/* SETMSB_3 only            */
	/*  167 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
	/*  168 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

				/* Mode 10 only             */
	/*  169 */  CR( 0,  0,  0,  0,  0,  1,  0),     /*  Clear 5     */
				/* Mode 10 and 11           */
	/*  170 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
	/*  171 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
	/*  172 */  CR( 6,  2,  F0, 0,  1,  0,  0),     /*  F0 (6 MSBs) */
	/*  173 */  CR( 6,  2,  F1, 0,  1,  0,  0),     /*  F1 (6 MSBs) */
	/*  174 */  CR( 6,  2,  F2, 0,  1,  0,  0),     /*  F2 (6 MSBs) */
				/* SETMSB_3 only            */
	/*  175 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
	/*  176 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */
};

static const INT16 sp0256_df_idx[16 * 8] =
{
	/*  OPCODE 0000 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
	/*  OPCODE 1000 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
	/*  OPCODE 0100 */      17, 22,     17, 24,     25, 30,     25, 32,
	/*  OPCODE 1100 */      83, 94,     129,142,    97, 108,    145,158,
	/*  OPCODE 0010 */      83, 96,     129,144,    97, 110,    145,160,
	/*  OPCODE 1010 */      73, 77,     74, 77,     78, 82,     79, 82,
	/*  OPCODE 0110 */      33, 36,     34, 37,     38, 41,     39, 42,
	/*  OPCODE 1110 */      127,128,    127,128,    127,128,    127,128,
	/*  OPCODE 0001 */      1,  14,     1,  16,     1,  14,     1,  16,
	/*  OPCODE 1001 */      45, 56,     45, 58,     59, 70,     59, 72,
	/*  OPCODE 0101 */      161,166,    162,166,    169,174,    170,174,
	/*  OPCODE 1101 */      111,116,    111,118,    119,124,    119,126,
	/*  OPCODE 0011 */      161,168,    162,168,    169,176,    170,176,
	/*  OPCODE 1011 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
	/*  OPCODE 0111 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
	/*  OPCODE 1111 */      0,  0,      0,  0,      0,  0,      0,  0
};

/* ======================================================================== */
/*  BITREV32       -- Bit-reverse a 32-bit number.                            */
/* ======================================================================== */
static inline UINT32 bitrev32(UINT32 val)
{
	val = ((val & 0xFFFF0000) >> 16) | ((val & 0x0000FFFF) << 16);
	val = ((val & 0xFF00FF00) >>  8) | ((val & 0x00FF00FF) <<  8);
	val = ((val & 0xF0F0F0F0) >>  4) | ((val & 0x0F0F0F0F) <<  4);
	val = ((val & 0xCCCCCCCC) >>  2) | ((val & 0x33333333) <<  2);
	val = ((val & 0xAAAAAAAA) >>  1) | ((val & 0x55555555) <<  1);

	return val;
}

/* ======================================================================== */
/*  BITREV8       -- Bit-reverse a 8-bit number.                            */
/* ======================================================================== */
static inline UINT8 bitrev8(UINT8 val)
{
	val = ((val & 0xF0) >>  4) | ((val & 0x0F) <<  4);
	val = ((val & 0xCC) >>  2) | ((val & 0x33) <<  2);
	val = ((val & 0xAA) >>  1) | ((val & 0x55) <<  1);

	return val;
}

/* ======================================================================== */
/*  BITREVBUFF       -- Bit-reverse a buffer.                               */
/* ======================================================================== */
void bitrevbuff(UINT8 *buffer, unsigned int start, unsigned int length)
{
	for (unsigned int i = start; i < length; i++ )
		buffer[i] = bitrev8(buffer[i]);
}

/* ======================================================================== */
/*  SP0256_GETB  -- Get up to 8 bits at the current PC.                     */
/* ======================================================================== */
UINT32 getb( int len )
{
	UINT32 data;
	UINT32 d0, d1;

	/* -------------------------------------------------------------------- */
	/*  Fetch data from the FIFO or from the MASK                           */
	/* -------------------------------------------------------------------- */
	if (m_fifo_sel)
	{
		d0 = m_fifo[(m_fifo_tail    ) & 63];
		d1 = m_fifo[(m_fifo_tail + 1) & 63];

		data = ((d1 << 10) | d0) >> m_fifo_bitp;


		/* ---------------------------------------------------------------- */
		/*  Note the PC doesn't advance when we execute from FIFO.          */
		/*  Just the FIFO's bit-pointer advances.   (That's not REALLY      */
		/*  what happens, but that's roughly how it behaves.)               */
		/* ---------------------------------------------------------------- */
		m_fifo_bitp += len;
		if (m_fifo_bitp >= 10)
		{
			m_fifo_tail++;
			m_fifo_bitp -= 10;
		}
	} else
	{
		/* ---------------------------------------------------------------- */
		/*  Figure out which ROMs are being fetched into, and grab two      */
		/*  adjacent bytes.  The byte we're interested in is extracted      */
		/*  from the appropriate bit-boundary between them.                 */
		/* ---------------------------------------------------------------- */
		int idx0 = (m_pc    ) >> 3, d0; //???
		int idx1 = (m_pc + 8) >> 3, d1;

		d0 = m_rom[idx0 & 0xffff];
		d1 = m_rom[idx1 & 0xffff]; // was 0xffff

		data = ((d1 << 8) | d0) >> (m_pc & 7);

		m_pc += len;
	}

	/* -------------------------------------------------------------------- */
	/*  Mask data to the requested length.                                  */
	/* -------------------------------------------------------------------- */
	data &= ((1 << len) - 1);

//			printf("m_pc %d\n",m_pc);

	return data;
}

/* ======================================================================== */
/*  SP0256_MICRO -- Emulate the microsequencer in the SP0256.  Executes     */
/*                  instructions either until the repeat count != 0 or      */
/*                  the sequencer gets halted by a RTS to 0.                */
/* ======================================================================== */
void micro()
{
	UINT8  immed4;
	UINT8  opcode;
	UINT16 cr;
	int ctrl_xfer;
	int repeat;
	int i, idx0, idx1;

	/* -------------------------------------------------------------------- */
	/*  Only execute instructions while the filter is not busy.             */
	/* -------------------------------------------------------------------- */
	while (m_filt.rpt <= 0)
	{
		/* ---------------------------------------------------------------- */
		/*  If the CPU is halted, see if we have a new command pending      */
		/*  in the Address LoaD buffer.                                     */
		/* ---------------------------------------------------------------- */
		if (m_halted && !m_lrq)
		{
		  m_pc       = m_ald | (0x1000 << 3); // ored with 32768
			m_fifo_sel = 0;
			m_halted   = 0;
			m_lrq      = 0x8000;
			m_ald      = 0;
			for (i = 0; i < 16; i++)
				m_filt.r[i] = 0;
			//			m_drq_cb(1);
		}

		/* ---------------------------------------------------------------- */
		/*  If we're still halted, do nothing.                              */
		/* ---------------------------------------------------------------- */
		if (m_halted)
		{
			m_filt.rpt = 1;
			m_lrq      = 0x8000;
			m_ald      = 0;
			for (i = 0; i < 16; i++)
				m_filt.r[i] = 0;

			//			SET_SBY(1)

			return;
		}

		/* ---------------------------------------------------------------- */
		/*  Fetch the first 8 bits of the opcode, which are always in the   */
		/*  same approximate format -- immed4 followed by opcode.           */
		/* ---------------------------------------------------------------- */
		immed4 = getb(4);
		opcode = getb(4);
		repeat = 0;
		ctrl_xfer = 0;

		//		printf("$%.4X.%.1X: OPCODE %d%d%d%d.%d%d\n",
		//		(m_pc >> 3) - 1, m_pc & 7,
		//		!!(opcode & 1), !!(opcode & 2),
		//		!!(opcode & 4), !!(opcode & 8),
		//		!!(m_mode&4), !!(m_mode&2));

		/* ---------------------------------------------------------------- */
		/*  Handle the special cases for specific opcodes.                  */
		/* ---------------------------------------------------------------- */
		switch (opcode)
		{
			/* ------------------------------------------------------------ */
			/*  OPCODE 0000:  RTS / SETPAGE                                 */
			/* ------------------------------------------------------------ */
			case 0x0:
			{
				/* -------------------------------------------------------- */
				/*  If immed4 != 0, then this is a SETPAGE instruction.     */
				/* -------------------------------------------------------- */
				if (immed4)     /* SETPAGE */
				{
					m_page = bitrev32(immed4) >> 13;
				} else
				/* -------------------------------------------------------- */
				/*  Otherwise, this is an RTS / HLT.                        */
				/* -------------------------------------------------------- */
				{
					UINT32 btrg;

					/* ---------------------------------------------------- */
					/*  Figure out our branch target.                       */
					/* ---------------------------------------------------- */
					btrg = m_stack;

					m_stack = 0;

					/* ---------------------------------------------------- */
					/*  If the branch target is zero, this is a HLT.        */
					/*  Otherwise, it's an RTS, so set the PC.              */
					/* ---------------------------------------------------- */
					if (!btrg)
					{
						m_halted   = 1;
						m_pc       = 0;
						ctrl_xfer  = 1;
					} else
					{
						m_pc      = btrg;
						ctrl_xfer = 1;
					}
				}

				break;
			}

			/* ------------------------------------------------------------ */
			/*  OPCODE 0111:  JMP          Jump to 12-bit/16-bit Abs Addr   */
			/*  OPCODE 1011:  JSR          Jump to Subroutine               */
			/* ------------------------------------------------------------ */
			case 0xE:
			case 0xD:
			{
				int btrg;

				/* -------------------------------------------------------- */
				/*  Figure out our branch target.                           */
				/* -------------------------------------------------------- */
				btrg = m_page                     |
						(bitrev32(immed4)  >> 17) |
						(bitrev32(getb(8)) >> 21);
				ctrl_xfer = 1;

				/* -------------------------------------------------------- */
				/*  If this is a JSR, push our return address on the        */
				/*  stack.  Make sure it's byte aligned.                    */
				/* -------------------------------------------------------- */
				if (opcode == 0xD)
					m_stack = (m_pc + 7) & ~7;

				/* -------------------------------------------------------- */
				/*  Jump to the new location!                               */
				/* -------------------------------------------------------- */
				m_pc = btrg;
				break;
			}

			/* ------------------------------------------------------------ */
			/*  OPCODE 1000:  SETMODE      Set the Mode and Repeat MSBs     */
			/* ------------------------------------------------------------ */
			case 0x1:
			{
				m_mode = ((immed4 & 8) >> 2) | (immed4 & 4) | ((immed4 & 3) << 4);
				break;
			}

			/* ------------------------------------------------------------ */
			/*  OPCODE 0001:  LOADALL      Load All Parameters              */
			/*  OPCODE 0010:  LOAD_2       Load Per, Ampl, Coefs, Interp.   */
			/*  OPCODE 0011:  SETMSB_3     Load Pitch, Ampl, MSBs, & Intrp  */
			/*  OPCODE 0100:  LOAD_4       Load Pitch, Ampl, Coeffs         */
			/*  OPCODE 0101:  SETMSB_5     Load Pitch, Ampl, and Coeff MSBs */
			/*  OPCODE 0110:  SETMSB_6     Load Ampl, and Coeff MSBs.       */
			/*  OPCODE 1001:  DELTA_9      Delta update Ampl, Pitch, Coeffs */
			/*  OPCODE 1010:  SETMSB_A     Load Ampl and MSBs of 3 Coeffs   */
			/*  OPCODE 1100:  LOAD_C       Load Pitch, Ampl, Coeffs         */
			/*  OPCODE 1101:  DELTA_D      Delta update Ampl, Pitch, Coeffs */
			/*  OPCODE 1110:  LOAD_E       Load Pitch, Amplitude            */
			/*  OPCODE 1111:  PAUSE        Silent pause                     */
			/* ------------------------------------------------------------ */
			default:
			{
				repeat = immed4 | (m_mode & 0x30);
				break;
			}
		}
		if (opcode != 1) m_mode &= 0xF;

		/* ---------------------------------------------------------------- */
		/*  If this was a control transfer, handle setting "fifo_sel"       */
		/*  and all that ugliness.                                          */
		/* ---------------------------------------------------------------- */
		if (ctrl_xfer)
		{

			/* ------------------------------------------------------------ */
			/*  Set our "FIFO Selected" flag based on whether we're going   */
			/*  to the FIFO's address.                                      */
			/* ------------------------------------------------------------ */
			m_fifo_sel = m_pc == FIFO_ADDR;


			/* ------------------------------------------------------------ */
			/*  Control transfers to the FIFO cause it to discard the       */
			/*  partial decle that's at the front of the FIFO.              */
			/* ------------------------------------------------------------ */
			if (m_fifo_sel && m_fifo_bitp)
			{

				/* Discard partially-read decle. */
				if (m_fifo_tail < m_fifo_head) m_fifo_tail++;
				m_fifo_bitp = 0;
			}


			continue;
		}

		/* ---------------------------------------------------------------- */
		/*  Otherwise, if we have a repeat count, then go grab the data     */
		/*  block and feed it to the filter.                                */
		/* ---------------------------------------------------------------- */
		if (!repeat) continue;

		m_filt.rpt = repeat + 1;

		i = (opcode << 3) | (m_mode & 6);
		idx0 = sp0256_df_idx[i++];
		idx1 = sp0256_df_idx[i  ];

		//		assert(idx0 >= 0 && idx1 >= 0 && idx1 >= idx0);

		/* ---------------------------------------------------------------- */
		/*  Step through control words in the description for data block.   */
		/* ---------------------------------------------------------------- */
		for (i = idx0; i <= idx1; i++)
		{
			int len, shf, delta, field, prm, clra, clr5;
			INT8 value;

			/* ------------------------------------------------------------ */
			/*  Get the control word and pull out some important fields.    */
			/* ------------------------------------------------------------ */
			cr = sp0256_datafmt[i];

			len   = CR_LEN(cr);
			shf   = CR_SHF(cr);
			prm   = CR_PRM(cr);
			clra  = cr & CR_CLRA;
			clr5  = cr & CR_CLR5;
			delta = cr & CR_DELTA;
			field = cr & CR_FIELD;
			value = 0;

			/* ------------------------------------------------------------ */
			/*  Clear any registers that were requested to be cleared.      */
			/* ------------------------------------------------------------ */
			if (clra)
			{
				for (int j = 0; j < 16; j++)
					m_filt.r[j] = 0;

				m_silent = 1;
			}

			if (clr5)
				m_filt.r[B5] = m_filt.r[F5] = 0;

			/* ------------------------------------------------------------ */
			/*  If this entry has a bitfield with it, grab the bitfield.    */
			/* ------------------------------------------------------------ */
			if (len)
			{
				value = getb(len);
			}
			else
			{
				continue;
			}

			/* ------------------------------------------------------------ */
			/*  Sign extend if this is a delta update.                      */
			/* ------------------------------------------------------------ */
			if (delta)  /* Sign extend */
			{
				if (value & (1 << (len - 1))) value |= -1 << len;
			}

			/* ------------------------------------------------------------ */
			/*  Shift the value to the appropriate precision.               */
			/* ------------------------------------------------------------ */
			if (shf)
				value <<= shf;


			m_silent = 0;

			/* ------------------------------------------------------------ */
			/*  If this is a field-replace, insert the field.               */
			/* ------------------------------------------------------------ */
			if (field)
			{

				m_filt.r[prm] &= ~(~0 << shf); /* Clear the old bits.     */
				m_filt.r[prm] |= value;        /* Merge in the new bits.  */


				continue;
			}

			/* ------------------------------------------------------------ */
			/*  If this is a delta update, add to the appropriate field.    */
			/* ------------------------------------------------------------ */
			if (delta)
			{

				m_filt.r[prm] += value;

				continue;
			}

			/* ------------------------------------------------------------ */
			/*  Otherwise, just write the new value.                        */
			/* ------------------------------------------------------------ */
			m_filt.r[prm] = value;
		}

		/* ---------------------------------------------------------------- */
		/*  Special case:  Set PAUSE's equivalent period.                   */
		/* ---------------------------------------------------------------- */
		if (opcode == 0xF)
		{
			m_silent = 1;
			m_filt.r[1] = PER_PAUSE;
		}

		/* ---------------------------------------------------------------- */
		/*  Now that we've updated the registers, go decode them.           */
		/* ---------------------------------------------------------------- */
		lpc12_regdec(&m_filt);

		/* ---------------------------------------------------------------- */
		/*  Break out since we now have a repeat count.                     */
		/* ---------------------------------------------------------------- */
		break;
	}
}

   

	void main(){
	  sp0256_init();
	  reset();
	int output_index = 0;
	  INT16 output[128];
	  int length, do_samp=1, did_samp, samples=32;
	  int data=0x0;
	  unsigned char happy=0;

	  while(1){

	    //	  reset();

	    if (m_filt.rpt <= 0){
	      //	      printf("NEW\n");
	    data=HAPPIER[happy%7];
	    happy++;
	    if (happy==4) {
	      happy=0;
	      reset();
	    }
	    m_lrq = 0; //from 8 bit write
	    m_ald = ((0xff & data) << 4); // gives us max 4080 - rom starts at 2048=0x1000
	      micro();
	    // static inline int lpc12_update(struct lpc12_t *f, int num_samp, INT16 *out, UINT32 *optr)
	    }
	    lpc12_update(&m_filt, do_samp, output, &m_sc_head);
	  }
	}


	  /*	while (output_index < samples)
	{

		while (m_sc_tail != m_sc_head)
		{
		  output[output_index++] = m_scratch[m_sc_tail++ & SCBUF_MASK];
			m_sc_tail &= SCBUF_MASK;

			if (output_index > samples)
				break;
		}

		if (output_index > samples)
			break;

		length = samples - output_index;
		did_samp = 0;
		//old_idx  = m_sc_head;
		if (length > 0) do
		{
			int do_samp;

			if (m_filt.rpt <= 0)
				micro();
			do_samp = length - did_samp;
			if (m_sc_head + do_samp - m_sc_tail > SCBUF_SIZE)
				do_samp = m_sc_tail + SCBUF_SIZE - m_sc_head;

			if (do_samp == 0) break;

			if (m_silent && m_filt.rpt <= 0)
			{
				int y = m_sc_head;

				for (int x = 0; x < do_samp; x++)
					m_scratch[y++ & SCBUF_MASK] = 0;
				m_sc_head += do_samp;
				did_samp    += do_samp;
			}
			else
			{
			  //				did_samp += lpc12_update(&m_filt, do_samp, m_scratch.get(), &m_sc_head);
			  // THE GET here does what in scratchbuf?
			}

			m_sc_head &= SCBUF_MASK;

		} while (m_filt.rpt >= 0 && length > did_samp);
	}
	  */


	    /*


Add(HH1)
	Add(AE)
	Add(PP)
	Add(YR)
	Add(P200ms)
	Add(P200ms)
	Add(P200ms)

	    */


