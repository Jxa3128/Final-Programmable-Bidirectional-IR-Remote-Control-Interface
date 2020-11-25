//Jorge Avila
//1001543128
#ifndef SPEAKER_TX_H_
#define SPEAKER_TX_H_
#include "tm4c123gh6pm.h"
#include <stdbool.h>
#include <stdint.h>

#define LoadValue 10000
//speaker transmitter TX -> PE4
#define speakerTX    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 4*4))) //bit 4
#define speakerTX_MASK  16//2^4

void initSPEAKER_TX(void);
void playSound();

void BomSom();
void SomRuim();
void playAlert_Good();
void playAlert_Bad(uint16_t hz, uint16_t d);

#endif
