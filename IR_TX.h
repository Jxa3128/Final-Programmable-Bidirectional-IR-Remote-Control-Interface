//Jorge Avila
#ifndef IR_TX_H_
#define IR_TX_H_
#include "tm4c123gh6pm.h"
#include <stdbool.h>
#include <stdint.h>

void timerInterrupt0(void);

#define DutyCycle 1050
#define initLOW 4500
#define initHIGH 9000 + 250
#define periodt 562 + 2

#define GPO_PWM    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 6*4))) //bit 6
#define GPO_PWM_MASK 64 //2^6

void initIR_TX(void);
void playComment(uint8_t address, uint8_t data);
void sendData(uint8_t address[8], uint8_t data[8]);
void addByte(uint8_t byte[8], bool);
void settimerInterrupt0(uint32_t us);
void timerInterrupt0();

#endif

