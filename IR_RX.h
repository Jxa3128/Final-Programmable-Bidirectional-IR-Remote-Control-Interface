//Jorge Avila 1001543128
#ifndef IR_RX_H_
#define IR_RX_H_
//lab 6 is here

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

// Bitband Aliases for input and output
#define IR_RX    (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 5*4))) //the actual signal
#define IR_TX    (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 0*4))) //the debugger

// PortB masks
#define IR_RX_MASK 32
#define IR_TX_MASK 1

// Samples Numbers
#define MAX_SAMPLES 102             // total number of samples that we need
#define INIT_SAMPLES 6              // size of the first 6 samples



void settimerInterrupt0(uint32_t );
void settimerInterrupt1(uint32_t);
void timerInterrupt1(void);
uint8_t getBit(void);
uint8_t invertBit(uint8_t);
bool checkError(void);
uint8_t bToI(uint8_t[]);
uint8_t getButton(void);
void parseBuffer(void);
void fallingEdge(void);
uint8_t BinarytoDecimal(uint8_t []);
void initIR_RX(void);

#endif
