//Jorge Avila
//mavID: 1001543128

/*
 *
 * The goal of this project is to build a device that serves a bidirectional IR interface,
 * The capable of learning and playing NEC format commands. The device can be used in later
 * The classes as an IoT device after adding a wireless transceiver, allowing remote control
 * The to act as a publisher and subscriber in an MQTT-based IoT system.
 * The Most parts for the project will be provided in ERB 126. The components of this project
 * The are presented in detail in the lectures and lab exercises.
 *
 *
 *
 *
 *
 *
 */

#include "IR_RX.h"
#include "IR_TX.h"
#include "SPEAKER_TX.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "eeprom.h"

enum NumeroDeBoton
{
    NotWorking = 0, Working = 1
};
#define BTNSIZE 21
//uint8_t decodeButton;
uint8_t botones[] = { 162, 98, 226, 34, 2, 194, 224, 168, 144, 104, 152, 176,
                      48, 24, 122, 16, 56, 90, 66, 74, 82 };
//
//uint64_t btn_names[7][3] = { { "CH-", "CH", "CH+" }, { "PREV", "NEXT",
//                                                       "PLAY/PAUSE" },
//                             { "VOL-", "VOL+", "EQ" }, { "0", "100+", "200+" },
//                             { "1", "2", "3" }, { "4", "5", "6" }, { "7", "8",
//                                                                     "9" } };
//prototypes
void printHelp();
void initHw();

//main
int main(void)
{
    initHw();
    initSPEAKER_TX();
    initIR_TX();
    initIR_RX();
    initUart0();
    initIR_TX();
    initEeprom();
    setUart0BaudRate(115200, 40e6);
    USER_DATA data;
    putsUart0(
            "\n\tWelcome to the final project -- type help for information.\n");
    while (true)
    {
        //GET THE DATA from user
        putsUart0("\n>> ");
        getsUart0(&data);
        putsUart0("\n");
        //parse the data
        parseFields(&data);

        /*
         uint8_t i;
         int32_t value = 0;
         for (i = 0; i < data.fieldCount; i++)
         {
         putcUart0(data.fieldType[i]);
         putcUart0('\t');
         putsUart0(getFieldString(&data, i));
         putcUart0('\n');
         }

         */
        //first initialize valid to false
        //then if it goes to an if -> set to true
        bool valid = false;
        if (isCommand(&data, "set", 2))
        {
            int32_t add_i = getFieldInteger(&data, 1);
            int32_t data_i = getFieldInteger(&data, 2);
            valid = true;
        }
        //alert good on or alert good off
        if (isCommand(&data, "alert", 1))
        {
            uint8_t i = 0;
            char *_status = getFieldString(&data, 3);
            if (myCompare(_status, "on"))
            {
                for (i = 0; i < 3; i++)
                    BomSom();
            }
            else
            {
                for (i = 0; i < 5; i++)
                    SomRuim();
            }
            valid = true;
        }
        if (isCommand(&data, "sendb", 1))
        {
            putsUart0("\nButton: ");
            uint8_t buttonNumber = getFieldInteger(&data, 1);
            ATOI(buttonNumber);
            putsUart0(" was sent.\n");
            playComment(0, botones[buttonNumber - 1]);
            valid = true;

        }
        if (isCommand(&data, "decode", 0))
        {
            isDecode(true);

            if (isCommand(&data, "decode", 1))
            {
                char *off = getFieldString(&data, 1);
                if (myCompare(off, "off"))
                {
                    isDecode(false);
                }
            }
            valid = true;
        }
        //min has to have one argument
        if (isCommand(&data, "learn", 1))
        {
            //if ex: learn NAME 0 162
            if (isCommand(&data, "learn", 3))
            {
                //ex: learn plusBtn 0 162
                char *name = getFieldString(&data, 1);
                uint8_t address = getFieldInteger(&data, 2);
                uint8_t dat = getFieldInteger(&data, 3);
                addInstruction(name, address, dat);
                valid = true;
            }
            //if ex: learn NAME
            else
            {

            }
        }
        if (isCommand(&data, "info", 1))
        {
            //if they type something like info 0
            if (data.fieldType[1] == 'N')
            {
                putsUart0("info: Integer\n");
                uint16_t position = getFieldInteger(&data, 1);
                infoIndex(position);
                valid = true;
            }
            //if user types something like info NAME
            if (data.fieldType[1] == 'A')
            {
                putsUart0("info: String\n");
                char *nombre = getFieldString(&data, 1);
                infoName(nombre);
                valid = true;
            }
        }
        if (isCommand(&data, "list", 1))
        {

            putsUart0("Printing the commands: \r\n");
            listCommands();
            valid = true;

        }
        if (isCommand(&data, "erase", 1))
        {
            char *name = getFieldString(&data, 1);
            eraseName(name);
            valid = true;
        }
        if (isCommand(&data, "play", 1))
        {
            valid = true;
        }
        if (isCommand(&data, "clear", 0))
        {
            clearEeprom();
            valid = true;
        }
        if (isCommand(&data, "help", 0))
        {
            printHelp();
            valid = true;
        }
        /*
         if (isCommand(&data, "buttons", 0))
         {
         uint8_t i = 0, j = 0;
         putsUart0("The buttons are the following: \n");
         for (i = 0; i < 7; i++)
         {
         for (j = 0; j < 3; j++)
         {

         putsUart0(btn_names[i]);
         if (i % 3 == 0)
         putcUart0('\n');
         }
         }

         valid = true;
         }
         */
        if (!valid)
        {
            putsUart0("Invalid command\n");
        }
    }
}

// Initialize Hardware
void initHw()
{
// Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN
            | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
    _delay_cycles(3);

}
void printHelp()
{
    putsUart0("\t\r\nWelcome to Embedded Systems I.\r\n");
    putsUart0("\tThe options are shown below: \n");
    putsUart0("\tdecode: shows you the address and data of pushed remote.\n");
    putsUart0(
            "\tlearn: learns the name of a button and save its data and address.\n");
    putsUart0(
            "\tinfo: prints out the saved names with its address and data.\n");
    putsUart0("\terase: erases the name with its data and address.\n");
    putsUart0("\tclear: clears the eeprom.\n");
    putsUart0(
            "\tplay: plays the command that is currently saved in the eeprom.\n");
    putsUart0("\thelp: shows the commands again.\r\n");

}
