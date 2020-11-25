/*
 * eeprom.c
 *
 *  Created on: Nov 25, 2020
 *      Author: jorgejuarez
 */

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "eeprom.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initEeprom()
{
    SYSCTL_RCGCEEPROM_R = 1;
    _delay_cycles(3);
    while (EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING)
        ;
}

void writeEeprom(uint16_t add, uint32_t data)
{
    EEPROM_EEBLOCK_R = add >> 4;
    EEPROM_EEOFFSET_R = add & 0xF;
    EEPROM_EERDWR_R = data;
    while (EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING)
        ;
}

uint32_t readEeprom(uint16_t add)
{
    EEPROM_EEBLOCK_R = add >> 4;
    EEPROM_EEOFFSET_R = add & 0xF;
    return EEPROM_EERDWR_R;
}
void clearEeprom()
{
    //could iterate thru a size of uint16_t
    writeEeprom(0, 0);
}
//brings in the name, the address and the data of the remote button
void addInstruction(char *name, uint8_t address, uint8_t data)
{
    //current size of how many things are saved
    uint16_t currentSize = readEeprom(0);
    uint8_t i;
    uint32_t tempArray[(STRSIZE / 4) + 1];
    //only iterated 3 times
    //clear everything (so set to 0)
    for (i = 0; i < (STRSIZE / 4) + 1; i++)
    {
        tempArray[i] = 0;
    }
    for (i = 0; i < (STRSIZE) && name[i] != '\0'; i++)
    {
        //8 goes into 32 four times -- never changes
        tempArray[i / 4] |= name[i] << ((4 - 1) - (i % 4)) * 8;
    }
    //tryna put in the last position in that block
    tempArray[(STRSIZE / 4) + 1 - 1] = (address << 8) | data;

    for (i = (currentSize * ((STRSIZE / 4) + 1)) + 1; i < (STRSIZE / 4) + 1;
            i++)
    {
        writeEeprom(i, tempArray[i]);
    }
    //increment to next name?
    writeEeprom(0, currentSize + 1);
}
void testCommand(){

}
