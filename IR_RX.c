//Jorge Avila
#include "IR_RX.h"
#include "SPEAKER_TX.h"
#include "uart0.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tm4c123gh6pm.h"

//if 0 bad, if 1 good
bool eSom;
uint8_t decodeButton = 0;
bool decode = false;
uint8_t testIndex;
bool bufferIR[MAX_SAMPLES - INIT_SAMPLES];

uint8_t sampleNum = 0;
uint8_t addr[8];
uint8_t data[8];
uint8_t addri[8];
uint8_t datai[8];

void initIR_RX()
{

    //was added from init IR_TX
    GPIO_PORTB_DIR_R &= ~IR_RX_MASK;
    GPIO_PORTB_DEN_R |= IR_RX_MASK;

    // Configure IR pins -- this was moved -- this was moved from initHw()
    GPIO_PORTB_DIR_R |= IR_TX_MASK;

    GPIO_PORTB_DR2R_R |= IR_TX_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTB_DEN_R |= IR_TX_MASK;

    // Configure falling edge interrupts on row inputs
    // (edge mode, single edge, falling edge, clear any interrupts, turn on)
    GPIO_PORTB_IS_R &= ~IR_RX_MASK;         //turning on falling edge interrupts
    GPIO_PORTB_IBE_R &= ~IR_RX_MASK;
    GPIO_PORTB_IEV_R &= ~IR_RX_MASK;
    GPIO_PORTB_ICR_R |= IR_RX_MASK;
    NVIC_EN0_R |= 1 << (INT_GPIOB - 16); // turn-on interrupt 17 (GPIOB) -> TABLE 2.9 PG 104
    GPIO_PORTB_IM_R |= IR_RX_MASK;                   // turn-on interrupt mask
    IR_TX = 1;
}

// Configure Timer 1
void settimerInterrupt1(uint32_t us)
{
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN; // turn-off counter before reconfiguring, =1
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER; // configure as 32-bit counter (A + B) -> CFG register, page 728, TIMER1_CFG_R = 0; is the same thing
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_1_SHOT; // One shot mode (TAMR reg page 732), or = 1 -> same thing
    TIMER1_TAILR_R = 40 * us;                 // 40 cycles per micro sec
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                // turn-on interrupts, = 1
    NVIC_EN0_R |= 1 << (INT_TIMER1A - 16);     // turn-on interrupt 37, page 104
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn-on timer, = 1
}

void timerInterrupt1()
{
    TIMER1_ICR_R |= 1; // clear interrupt every time fn is called so it doesn't repeat itself

    // Test pts
    uint32_t deltaSamples[] = { 2250, 2250, 2250, 3750, 1500, 1781 + 250 };
    //was 562
    uint32_t widthSample = 562 + 2;
    IR_TX ^= 1;                                  // for test on the oscilloscope
    bool error = false;

    if (testIndex < MAX_SAMPLES)
    {
        if (testIndex < INIT_SAMPLES)
        {
            switch (testIndex)
            {
            case 1:
            case 2:
            case 3:
                if (IR_RX != 0)                  // Not Low
                {
                    error = true;
                    //bad sound
                    eSom = false;
                    testIndex = MAX_SAMPLES;
                    //SomRuim();
                }
                break;
            case 4:
            case 5:
                if (IR_RX != 1)                  // Not High
                {
                    eSom = false;
                    //SomRuim();
                    error = true;
                    testIndex = MAX_SAMPLES;
                }
                break;
            }
            if (!error)
                settimerInterrupt1(deltaSamples[testIndex]);
        }
        else
        {
            settimerInterrupt1(widthSample);
            bufferIR[testIndex - INIT_SAMPLES] = IR_RX;
            /*
             if (IR_RX)
             putcUart0('1');
             else
             putcUart0('0');
             */
        }
        testIndex++;
    }
    else
    {
        //putcUart0('\n');
        parseBuffer();
        GPIO_PORTB_ICR_R |= IR_RX_MASK;         // clear interrupt
        GPIO_PORTB_IM_R |= IR_RX_MASK;    // turn falling edge interrupt back on
    }

    if (error)                                   // Frame preamble is wrong
    {
        TIMER1_CTL_R &= ~TIMER_CTL_TAEN;
        GPIO_PORTB_ICR_R |= IR_RX_MASK;         // clear interrupt
        GPIO_PORTB_IM_R |= IR_RX_MASK;    // turn falling edge interrupt back on
        eSom = false;
        //SomRuim();
        return;
    }
}

// Modified _ccs.c file too
void fallingEdge()
{
    GPIO_PORTB_ICR_R |= IR_RX_MASK;                  // clear interrupt
    GPIO_PORTB_IM_R &= ~IR_RX_MASK;             // turn-off key press interrupts

    testIndex = 0;                                   // start
    timerInterrupt1();                                // call interrupt fn
}

uint8_t invertBit(uint8_t bit)
{
    if (bit == 0)
        return 1;
    if (bit == 1)
        return 0;
    return 255;
}

// Binary to decimal/Integer conversion
uint8_t bToI(uint8_t byte[8])
{
    uint8_t i;
    uint8_t mult = 1;
    uint8_t result = 0;

    for (i = 0; i < 8; i++)
    {
        result += byte[7 - i] * mult;
        mult *= 2;
    }
    return result;
}
//my version of Binary to Decimal
uint8_t BinarytoDecimal(uint8_t A[8])
{
    uint8_t i = 0, res;
    for (i = 0; i < 8; i++)
    {
        res += A[i] << i;
    }
    return res;

}
void putiUart0(uint8_t num)
{
    uint8_t countValue = num;
    uint8_t digits = 0;
    while (countValue > 0)
    {
        countValue /= 10;
        digits++;
    }
    if (digits == 0)
    {
        putcUart0('0');
    }
    else
    {
        char ans[4] = "\0\0\0\0";
        for (digits = digits; digits > 0; digits--)
        {
            ans[digits - 1] = (num % 10) + '0';
            num /= 10;
        }
        putsUart0(ans);
    }
}
//check if the error is present
bool checkError()
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        // Check correct bit
        if (!(addr[i] == 0 || addr[i] == 1))
            return true;
        if (!(addri[i] == 0 || addri[i] == 1))
            return true;
        if (!(data[i] == 0 || data[i] == 1))
            return true;
        if (!(datai[i] == 0 || datai[i] == 1))
            return true;
        // Check the complement
        if (addr[i] != invertBit(addri[i]))
            return true;
        if (data[i] != invertBit(datai[i]))
            return true;
    }
    return false;
}

// 01 = 0, 0111 = 1
uint8_t getBit()
{
    if (bufferIR[sampleNum] == 0 && bufferIR[sampleNum + 1] == 1
            && bufferIR[sampleNum + 2] == 0) // _| |_
    {
        sampleNum += 2;
        //putcUart0('0');
        return 0;
    }
    if (bufferIR[sampleNum] == 0 && bufferIR[sampleNum + 1] == 1
            && bufferIR[sampleNum + 2] == 1 && bufferIR[sampleNum + 3] == 1
            && bufferIR[sampleNum + 4] == 0)                    // _|    |_
    {
        sampleNum += 4;
        //putcUart0('1');
        return 1;
    }
    //putcUart0('E');
    return 255;
}

// Match data with respective button number
uint8_t getButton()
{
    uint8_t buttons[] = { 162, 98, 226, 34, 2, 194, 224, 168, 144, 104, 152,
                          176, 48, 24, 122, 16, 56, 90, 66, 74, 82 };
    uint8_t i;
    if (bToI(addr) == 0)
    {
        for (i = 0; i < 21; i++)
        {
            if (bToI(data) == buttons[i])
                return i + 1;
        }

        return 255;     //invalid button
    }
    else
        return 254;     // invalid remote
}

// Test the samples and show the matching button
void parseBuffer()
{
    //uint8_t i;

    uint8_t bitNum = 0;
    sampleNum = 0;
    for (bitNum = 0; bitNum < 32; bitNum++)
    {
        if (bitNum < 8)
            addr[bitNum] = getBit();
        if (bitNum >= 8 && bitNum < 16)
            addri[bitNum - 8] = getBit();
        if (bitNum >= 16 && bitNum < 24)
            data[bitNum - 16] = getBit();
        if (bitNum >= 24 && bitNum < 32)
            datai[bitNum - 24] = getBit();
    }
    //putcUart0('\n');

    if (!checkError())
    {
        uint8_t button = getButton();
//        decodeButton = button;
        if (button < 253)
        {
            uint8_t bns[] = { 162, 98, 226, 34, 2, 194, 224, 168, 144, 104, 152,
                              176, 48, 24, 122, 16, 56, 90, 66, 74, 82 };
            putsUart0("Button ");
            putiUart0(button);
            eSom = true;
            //good sound
            BomSom();
            //if decode is selected from main
            if (decode)
            {
                uint8_t iterator;
                uint8_t tempAdd[8], tempData[8];
                putsUart0("\nAddress: ");
                for (iterator = 8; iterator > 0; iterator--)
                {
                    tempAdd[8 - iterator] = (1 << (iterator - 1)) & 0;
                    if (tempAdd[8 - iterator])
                    {
                        putcUart0('1');
                    }
                    else
                    {
                        putcUart0('0');
                    }
                }
                putcUart0('\n');
                putsUart0("Data: ");

                //it is getting the correct buttin address
                //putiUart0(bns[button - 1]);
                for (iterator = 8; iterator > 0; iterator--)
                {
                    tempData[8 - iterator] = (1 << (iterator - 1))
                            & bns[button - 1];
                    if (tempData[8 - iterator])
                    {
                        putcUart0('1');
                    }
                    else
                    {
                        putcUart0('0');
                    }
                }
                putcUart0('\n');
            }
        }
        else
        {
            //if bad sound
            if (eSom == 0)
            {
                SomRuim();
            }
            if (button == 255)
                putsUart0("Invalid Button.");
            if (button == 254)
                putsUart0("Invalid Remote.");
            if (button == 253)
                putsUart0("Error.");
        }
        putcUart0('\n');
    }
    else
    {
        if (eSom == 0)
        {
            SomRuim();
        }
    }
}

uint8_t getBtn(uint8_t _btn)
{
    return _btn;
}
void isDecode(bool d)
{
    decode = d;
}
