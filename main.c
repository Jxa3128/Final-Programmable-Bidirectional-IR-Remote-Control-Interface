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

enum NumeroDeBoton
{
    NotWorking = 0, Working = 1
};

//prototypes
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
    setUart0BaudRate(115200, 40e6);
    USER_DATA data;
    while (true)
    {
        //GET THE DATA from user
        getsUart0(&data);
        //putsUart0("\n");
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
        if (isCommand(&data, "alert", 1))
        {
            char *str = getFieldString(&data, 1);
            valid = true;
        }
        if (isCommand(&data, "SENDB", 1))
        {
            putsUart0("\nButton: ");
            uint8_t botones[] =
                    { 162, 98, 226, 34, 2, 194, 224, 168, 144, 104, 152, 176,
                      48, 24, 122, 16, 56, 90, 66, 74, 82 };
            uint8_t buttonNumber = getFieldInteger(&data, 1);
            putiUart0(buttonNumber);
            putsUart0(" was sent.\n");
            //uint8_t it = 0;
            playComment(0, botones[buttonNumber - 1]);
            //playComment(0, 162);
            valid = true;

        }
        if (isCommand(&data, "PLAY", 0))
        {

        }
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
