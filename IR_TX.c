//Jorge Avila 1001543128
//this is the transmitter IR sensor
// Modified _ccs.c file too
#include "IR_TX.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>

uint8_t index = 0;
bool output[100];

// Configure Timer 0
void settimerInterrupt0(uint32_t us)
{
    TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // turn-off counter before reconfiguring, =1
    TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER; // configure as 32-bit counter (A + B) -> CFG register, page 728, TIMER1_CFG_R = 0; is the same thing
    TIMER0_TAMR_R = TIMER_TAMR_TAMR_1_SHOT; // One shot mode (TAMR reg page 732), or = 1 -> same thing
    TIMER0_TAILR_R = 40 * us;                 // 40 cycles per micro sec
    TIMER0_IMR_R = TIMER_IMR_TATOIM;                // turn-on interrupts, = 1
    NVIC_EN0_R |= 1 << (INT_TIMER0A - 16);     // turn-on interrupt 37, page 104
    TIMER0_CTL_R |= TIMER_CTL_TAEN;                 // turn-on timer, = 1
}

void timerInterrupt0()
{
    TIMER0_ICR_R |= 1;
    //putcUart0('_');
    if (index == 0)
    {
        GPIO_PORTB_DEN_R |= GPO_PWM_MASK;
        settimerInterrupt0(initHIGH);
        //putsUart0("0");
    }
    if (index == 1)
    {
        GPIO_PORTB_DEN_R &= ~GPO_PWM_MASK;
        settimerInterrupt0(initLOW);
        //putsUart0("1");
    }
    if (index > 1 && index < 100)
    {
        if (output[index])
        {
            //putsUart0("1");
            GPIO_PORTB_DEN_R &= ~GPO_PWM_MASK;
        }
        else
        {
            //putsUart0("0");
            GPIO_PORTB_DEN_R |= GPO_PWM_MASK;
        }
        settimerInterrupt0(periodt);
    }
    index += 1;
}
//this was once named initPWM
void initIR_TX()
{

    //initalize
    //port B was initalized here
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R0 | SYSCTL_RCGCTIMER_R1;
    _delay_cycles(3);

    // Set GPIO ports to use APB -- this was moved from initHw()
    SYSCTL_GPIOHBCTL_R = 0;

    //configure the PWMO (Gen 0 - Output A)
    GPIO_PORTB_DIR_R |= GPO_PWM_MASK; //GPO_PWM_MASK;   // make bit 6 an output
    GPIO_PORTB_DR2R_R |= GPO_PWM_MASK; //GPO_PWM_MASK; // set drive strength to 2mA
    GPIO_PORTB_DEN_R |= GPO_PWM_MASK; //GPO_PWM_MASK;   // enable digital
    GPIO_PORTB_AFSEL_R |= GPO_PWM_MASK; //GPO_PWM_MASK;

    GPIO_PORTB_PCTL_R &= GPIO_PCTL_PB6_M;      // enable PWM
    GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB6_M0PWM0;

    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;
    SYSCTL_SRPWM_R = 0;
    PWM0_0_CTL_R = 0;
    PWM0_0_GENA_R |= PWM_0_GENA_ACTCMPBD_ZERO | PWM_0_GENA_ACTLOAD_ONE;
    PWM0_0_LOAD_R = DutyCycle;
    PWM0_INVERT_R = PWM_INVERT_PWM0INV;
    PWM0_0_CMPB_R = (DutyCycle / 2) - 1;
    PWM0_0_CTL_R = PWM_0_CTL_ENABLE;
    PWM0_ENABLE_R = PWM_ENABLE_PWM0EN;

    GPIO_PORTB_DEN_R &= ~GPO_PWM_MASK;
}

void playComment(uint8_t address, uint8_t data)
{
    uint8_t addressArray[8], dataArray[8];
    uint8_t i, test;
    test = address;
    //converting the address and data first to binary
    //before sending it at the bottom to
    //sendData(addressArray,dataArray);
    //putsUart0("Testing addressArray: ");
    for (i = 8; i > 0; --i)
    {
        addressArray[8 - i] = (1 << (i - 1)) & test;
        /*
         if(addressArray[8-i]){
         putcUart0('1');
         }else
         putcUart0('0');
         */
    }
    test = data;
    //putcUart0('\n');
    //putsUart0("Testing dataArray: ");
    for (i = 8; i > 0; --i)
    {
        dataArray[8 - i] = (1 << (i - 1)) & test;
        /*
         if(dataArray[8-i]){
         putcUart0('1');
         }else
         putcUart0('0');
         */
    }
    //putcUart0('\n');
    sendData(addressArray, dataArray);
}

//function: takes two uint8_t arrays -> addy & data
void sendData(uint8_t address[8], uint8_t data[8])
{
    //output is a global array variable
    index = 0;
    output[index++] = 1;
    output[index++] = 0;
    addByte(address, false);
    addByte(address, true);
    addByte(data, false);
    addByte(data, true);
    output[index++] = 0;
    output[index++] = 1;
    index = 0;
    //putcUart0('\n');
    timerInterrupt0();
}
void addByte(uint8_t byte[8], bool invert)
{
    uint8_t i;
    //putsUart0("Testing again: \n");
    for (i = 0; i < 8; i++)
    {
        /*
         putiUart0(i);
         putsUart0(": ");
         if (byte[i])
         putcUart0('1');
         else
         putcUart0('0');
         putsUart0(": ");
         */
        if (!invert)
        {
            if (byte[i])
            {
                output[index] = 0;
                output[index + 1] = 1;
                output[index + 2] = 1;
                output[index + 3] = 1;
                index += 4;
                //putsUart0("0111");
            }
            else
            {
                output[index] = 0; //0
                output[index + 1] = 1;
                index += 2;
                //putsUart0("01");
            }
        }
        else
        {
            if (!byte[i])
            {
                output[index] = 0;
                output[index + 1] = 1;
                output[index + 2] = 1;
                output[index + 3] = 1;
                index += 4;
                //putsUart0("0111");
            }
            else
            {
                output[index] = 0; //0
                output[index + 1] = 1;
                index += 2;
                //putsUart0("01");
            }
        }
        //putcUart0('\n');
    }
}
