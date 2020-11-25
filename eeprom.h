/*
 * eeprom.h
 *
 *  Created on: Nov 25, 2020
 *      Author: jorgejuarez
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#define STRSIZE 12
void initEeprom();
void writeEeprom(uint16_t add, uint32_t data);
uint32_t readEeprom(uint16_t add);
void addInstruction(char *, uint8_t, uint8_t);
void clearEeprom();


#endif /* EEPROM_H_ */