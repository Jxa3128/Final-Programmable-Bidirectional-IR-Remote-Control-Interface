//Jorge Avila 1001543128

// When the interrupt is called (or hit)
// Modified _ccs.c file too
#include "IR_TX.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>
#include "SPEAKER_TX.h"
#include "wait.h"

//inits all the components for the speaker
void initSPEAKER_TX()
{

    //initalize
    //port E was initalized here - M1
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;
    //port e
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    _delay_cycles(3);

    // Set GPIO ports to use APB -- this was moved from initHw()
    SYSCTL_GPIOHBCTL_R = 0;

    // Speaker on M1PWM2 (PE4), M1PWM1a
    //which means
    //generator 1
    //output a
    // position 2
    GPIO_PORTE_DIR_R |= speakerTX_MASK; //GPO_PWM_MASK;   // make bit 6 an output
    GPIO_PORTE_DR2R_R |= speakerTX_MASK; //GPO_PWM_MASK; // set drive strength to 2mA
    GPIO_PORTE_DEN_R |= speakerTX_MASK; //GPO_PWM_MASK;   // enable digital
    GPIO_PORTE_AFSEL_R |= speakerTX_MASK; //GPO_PWM_MASK;

    //I will be using M1PWM2
    GPIO_PORTE_PCTL_R &= GPIO_PCTL_PE4_M;
    GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_M1PWM2;

    //since M1 -> we use R1
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;
    SYSCTL_SRPWM_R = 0;
    PWM1_1_CTL_R = 0; //turn off generator 1
    PWM1_1_GENA_R |= PWM_1_GENA_ACTCMPBD_ZERO | PWM_1_GENA_ACTLOAD_ONE;
    PWM1_1_LOAD_R = LoadValue; //(clk freq/2)/desired freq
    PWM1_INVERT_R = PWM_INVERT_PWM2INV;
    PWM1_1_CMPB_R = (LoadValue / 2) - 1; //%50 DC
    PWM1_1_CTL_R = PWM_1_CTL_ENABLE;
    PWM1_ENABLE_R = PWM_ENABLE_PWM2EN;

    GPIO_PORTE_DEN_R &= ~speakerTX_MASK;

    playSound();
}

void playSound()
{
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R |= speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R &= ~speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R |= speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R &= ~speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R |= speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R &= ~speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R |= speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R &= ~speakerTX_MASK;
}
void playAlert_Good()
{

    GPIO_PORTE_DEN_R |= speakerTX_MASK;
    waitMicrosecond(1e5);
    PWM1_1_LOAD_R = 10000;
    GPIO_PORTE_DEN_R |= speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R &= ~speakerTX_MASK;
    waitMicrosecond(1e5);
}
void playAlert_Bad()
{

    GPIO_PORTE_DEN_R |= speakerTX_MASK;
    waitMicrosecond(1e5);
    PWM1_1_LOAD_R = 10000;
    GPIO_PORTE_DEN_R |= speakerTX_MASK;
    waitMicrosecond(1e5);
    GPIO_PORTE_DEN_R &= ~speakerTX_MASK;
    waitMicrosecond(1e5);
}
void BomSom()
{

    PWM1_1_LOAD_R = 8000; //(clk freq/2)/desired freq

    playAlert_Good();

}
void SomRuim()
{
    PWM1_1_LOAD_R = 7000; //(clk freq/2)/desired freq
    playAlert_Bad();
}

