/*
 * eeprom.c
 *
 *  Created on: Nov 25, 2020
 *      Author: jorgejuarez
 */

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "eeprom.h"
#include "uart0.h"

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
    putsUart0("Eeprom is cleared!\n");
}
//brings in the name, the address and the data of the remote button
void addInstruction(char *name, uint8_t address, uint8_t data)
{
    //current size of how many things are saved
    uint16_t currentSize = readEeprom(0);
    uint8_t i;
    uint32_t tempArray[(STRSIZE / 4) + 1];
    uint16_t st = (currentSize * ((STRSIZE / 4) + 1)) + 1;
    //only iterated 3 times
    //clear everything (so set to 0)
    for (i = 0; i < (STRSIZE / 4) + 1; i++)
    {
        tempArray[i] = 0; //acts like memset
    }
    for (i = 0; i < (STRSIZE) && name[i] != '\0'; i++)
    {
        //8 goes into 32 four times -- never changes
        tempArray[i / 4] |= name[i] << ((4 - 1) - (i % 4)) * 8;
    }
    //tryna put in the last position in that block
    tempArray[(STRSIZE / 4) + 1 - 1] = (address << 8) | data;


    for (i = 0; i < (STRSIZE / 4) + 1; i++)
    {
        writeEeprom(i + st, tempArray[i]);
    }
    //increment to next name?
    writeEeprom(0, currentSize + 1);
}
void testCommand()
{

}
//infoC
//this finds the command at that index (position)
void infoIndex(uint16_t index)
{
    uint32_t size = readEeprom(0);
    uint8_t it = 0;
    char name[STRSIZE];
    uint16_t st = (index * ((STRSIZE / 4) + 1)) + 1;
    //STRSIZE/4 + 1 -> 3+1 => 4
    //below should equal 4
    uint32_t tempArr[(STRSIZE / 4) + 1];
    //clear the tempArr to zeros..
    for (it = 0; it < (STRSIZE / 4) + 1; it++)
    {
        tempArr[it] = 0;
    }
    if (index < size)
    {
        for (it = 0; it < (STRSIZE / 4) + 1; it++)
        {
            tempArr[it] = readEeprom(st + it);
        }
        for (it = 0; it < STRSIZE; it++)
        {
            name[it] = (tempArr[it / 4] << ((it % 4)) * 8) >> 3 * 8;
        }
        putsUart0("The name is: ");
        putsUart0(name);
        putcUart0('\n');

        //now retrieving the address and the data
        uint8_t addy = (tempArr[((STRSIZE / 4) + 1) - 1] << 16) >> 24;
        uint8_t datty = (tempArr[((STRSIZE / 4) + 1) - 1] << 24) >> 24;

        putsUart0("The Address is: ");
        printDectoBin(addy);
        putcUart0('\n');
        putsUart0("The Data is: ");
        printDectoBin(datty);
        putcUart0('\n');

    }
    //not in range
    else
    {
        putsUart0("Invalid Index\n");
    }
}
//very good implementation of printing and
//converting decimal to binary
//thanks to buzzyFace --------========-------
void printDectoBin(uint8_t x)
{
    uint8_t i;
    for (i = 8; i > 0; i--)
    {
        if ((1 << (i - 1)) & x)
        {
            putcUart0('1');
        }
        else
        {
            putcUart0('0');
        }
    }
}
//void findInstruction(char *nombre, uint8_t Addy, uint8_t Datty)
//{
//
//}

