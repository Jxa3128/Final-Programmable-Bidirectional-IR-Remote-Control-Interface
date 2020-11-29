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
uint8_t currentStatus;

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
    uint32_t currentSize = readEeprom(0);
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
    uint8_t i = 0;
    for (; i < (STRSIZE / 4) + 1; ++i)
    {
        putcUart0(readEeprom(i));
    }
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
//this is when the user says info NAME
void infoName(char *name)
{
    // first it is ideal that we get the index of said name
    currentStatus = notFound;
    uint8_t currentIndex = findIndex(name);
    if (currentStatus == Found)
    {
        infoIndex(currentIndex);
    }
    else if (currentStatus == notFound)
    {
        putsUart0("The name was not located.\n");
    }
    else
    {
        putsUart0("Error!\n");
    }

}
uint32_t findIndex(char *name)
{

    //this is the current size
    uint32_t sz = readEeprom(0);
    uint16_t position = 0;

    //iterate through the eeprom size *think phat array*
    for (position = 0; position < sz; position++)
    {
        //the st -> start
        //start is a variable that is changing through
        //each iteration
        int16_t st = (position * ((STRSIZE / 4) + 1)) + 1;

        uint8_t it = 0;
        //init tempArr
        uint32_t tempArr[(STRSIZE / 4) + 1];
        //clear tempArr
        for (it = 0; it < (STRSIZE / 4) + 1; it++)
        {
            tempArr[it] = 0;
        }
        bool isMatching = true;
        //start reading from the eeprom
        //and start putting them in each position
        //question (STRSIZE / 4) + 1
        for (it = 0; it < 4; it++)
        {
            tempArr[it] = readEeprom(st + it);
        }
        //iteratgye through the string size you implemented
        for (it = 0; it < STRSIZE; it++)
        {
            if ((tempArr[it / 4] << ((it % 4)) * 8) >> 3 * 8 != name[it])
            {
                //they are not the same
                isMatching = false;
            }
            if (name[it] == '\0')
                break;
        }
        if (isMatching == 1)
        {
            currentStatus = Found;
            return position;
        }

    }
    currentStatus = notFound;
    return notFound;
}
//this function now deletes the command
//by finding the location
//and rewriting to the eeprom
//this function now deletes the command
//by finding the location
//and rewriting to the eeprom
void eraseName(char *name)
{
    currentStatus = notFound;
    uint8_t currentIndex = findIndex(name);
    if (currentStatus == Found)
    {
        uint32_t sz;
        uint16_t st;
        uint8_t it = 0;
        //read the size
        sz = readEeprom(0);
        //find the start position using the index of
        //that name using the function
        st = (currentIndex * ((STRSIZE / 4) + 1)) + 1;

        //iterate and find -> writeEeprom();
        for (it = 0; it < (STRSIZE / 4) + 1; it++)
        {
            writeEeprom(st + it, 0);
        }
        putsUart0("The command: ");
        putsUart0(name);
        putsUart0(" has been removed.\n");
    }
    else if (currentStatus == notFound)
    {
        putsUart0("The name was not located.\n");
    }
    else
    {
        putsUart0("Error!\n");
    }
}
void listCommands()
{
    uint32_t sz;
    uint8_t position = 0;
    uint16_t st;
    uint8_t i = 0;
    uint32_t tempArr[(STRSIZE / 4) + 1];
    char nombre[STRSIZE];
    sz = readEeprom(0);
    for (position = 0; position < sz; position = position + 1)
    {
        st = (position * ((STRSIZE / 4) + 1)) + 1;

        //clear the temp array
        for (i = 0; i < (STRSIZE / 4) + 1; i++)
        {
            tempArr[i] = 0;
        }
        //this tempArr holds the extraction of the name

        for (i = 0; i < (STRSIZE / 4) + 1; i++)
        {
            tempArr[i] = readEeprom(st + i);
        }
        for (i = 0; i < STRSIZE; i++)
        {
            nombre[i] = (tempArr[i / 4] << ((i % 4)) * 8) >> 3 * 8;

        }
        if (nombre[0] != '\0')
        {
            putsUart0("\n\t");
            putsUart0(nombre);
            uint8_t _address = (tempArr[(STRSIZE / 4)] << 16) >> 24;
            uint8_t _data = (tempArr[(STRSIZE / 4)] << 24) >> 24;
            putsUart0("\tAddress is: ");
            printDectoBin(_address);
            putsUart0("\t");
            putsUart0("Data is: ");
            printDectoBin(_data);
        }

    }
}
/*
 *void eraseName(char *name)
 {
 currentStatus = notFound;
 uint8_t currentIndex = findIndex(name);
 if (currentStatus == Found)
 {
 uint32_t tempArr[(STRSIZE / 4) + 1] = { 0, 0, 0, 0 };
 uint32_t sz;
 uint16_t st,newSt;
 uint8_t it = 0;
 uint16_t position = 0;

 //read the size
 sz = readEeprom(0);
 //find the start position using the index of
 //that name using the function
 st = (currentIndex * ((STRSIZE / 4) + 1)) + 1;
 //read the eeprom into a temp array
 for (it = 0; it < (STRSIZE / 4) + 1; it++)
 {
 tempArr[it] = readEeprom(st + it);
 }
 for (position = 0; position < sz; position++)
 {
 newSt = (position * ((STRSIZE / 4) + 1)) + 1;
 for (it = 0; it < (STRSIZE / 4) + 1; it++)
 {
 if ((tempArr[it / 4] << ((it % 4)) * 8) >> 3 * 8 == name[it])
 {
 tempArr[it] = 0; //try to delete it?
 }
 else
 {
 tempArr[it] = readEeprom(newSt + it);
 }

 }
 }
 for (it = 0; it < (STRSIZE / 4) + 1; it++)
 {
 if ((tempArr[it / 4] << ((it % 4)) * 8) >> 3 * 8 != 0)
 writeEeprom(it + st, tempArr[it]);
 }
 //bool isValid = tempArr[4] >> 31;
 putsUart0("\nis Valid is: ");
 //putcUart0(isValid);
 //iterate and find -> writeEeprom();
 for (it = 0; it < (STRSIZE / 4) + 1; it++)
 {
 writeEeprom(st + it, 0);
 }
 sz = sz - 1;
 writeEeprom(0, sz);
 putsUart0("The command: ");
 putsUart0(name);
 putsUart0(" has been removed.\n");
 }
 else if (currentStatus == notFound)
 {
 putsUart0("The name was not located.\n");
 }
 else
 {
 putsUart0("Error!\n");
 }
 }
 *
 *
 *
 */

