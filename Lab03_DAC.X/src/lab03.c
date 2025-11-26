#include "lab03.h"

#include <xc.h>
//do not change the order of the following 2 definitions
#define FCY 12800000UL
#include <libpic30.h>

#include "types.h"
#include "lcd.h"
#include "led.h"

/*
 * DAC code
 */

#define DAC_CS_TRIS TRISDbits.TRISD8
#define DAC_SDI_TRIS TRISBbits.TRISB10
#define DAC_SCK_TRIS TRISBbits.TRISB11
#define DAC_LDAC_TRIS TRISBbits.TRISB13
    
#define DAC_CS_PORT PORTDbits.RD8
#define DAC_SDI_PORT PORTBbits.RB10
#define DAC_SCK_PORT PORTBbits.RB11
#define DAC_LDAC_PORT PORTBbits.RB13

#define DAC_SDI_AD1CFG AD1PCFGLbits.PCFG10
#define DAC_SCK_AD1CFG AD1PCFGLbits.PCFG11
#define DAC_LDAC_AD1CFG AD1PCFGLbits.PCFG13

#define DAC_SDI_AD2CFG AD2PCFGLbits.PCFG10
#define DAC_SCK_AD2CFG AD2PCFGLbits.PCFG11
#define DAC_LDAC_AD2CFG AD2PCFGLbits.PCFG13

void dac_initialize()
{
    // set AN10, AN11 AN13 to digital mode
    SETBIT(DAC_SDI_AD1CFG);
    SETBIT(DAC_SCK_AD1CFG);
    SETBIT(DAC_LDAC_AD1CFG);

    SETBIT(DAC_SDI_AD2CFG);
    SETBIT(DAC_SCK_AD2CFG);
    SETBIT(DAC_LDAC_AD2CFG);
    
    // set RD8, RB10, RB11, RB13 as output pins
    CLEARBIT(DAC_CS_TRIS);
    CLEARBIT(DAC_SDI_TRIS);
    CLEARBIT(DAC_SCK_TRIS);
    CLEARBIT(DAC_LDAC_TRIS);
    
    // set default state: CS=1, SCK=0, SDI=0, LDAC=1
    SETBIT(DAC_CS_PORT);
    CLEARBIT(DAC_SCK_PORT);
    CLEARBIT(DAC_SDI_PORT);
    SETBIT(DAC_LDAC_PORT);
}

/*
 * Timer code
 */

#define FCY_EXT   32768UL

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

volatile uint32_t interrupt_counter = 0;

void timer_initialize()
{
    // Enable RTC Oscillator -> this effectively does OSCCONbits.LPOSCEN = 1
    // but the OSCCON register is lock protected. That means you would have to 
    // write a specific sequence of numbers to the register OSCCONL. After that 
    // the write access to OSCCONL will be enabled for one instruction cycle.
    // The function __builtin_write_OSCCONL(val) does the unlocking sequence and
    // afterwards writes the value val to that register. (OSCCONL represents the
    // lower 8 bits of the register OSCCON)
    __builtin_write_OSCCONL(OSCCONL | 2);
    // configure timer

    CLEARBIT(T1CONbits.TON); 
    SETBIT(T1CONbits.TCS); 
    T1CONbits.TCKPS = 0b00;
    T1CONbits.TCS=1; //external 32kHz
    TMR1 = 0;

    PR1 = 31;

    IFS0bits.T1IF = 0;
    IPC0bits.T1IP = 0x01;
    SETBIT(IEC0bits.T1IE);
    SETBIT(T1CONbits.TON);
}

void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    interrupt_counter++;
}

/*
 * main loop
 */

void main_loop()
{
    lcd_printf("Lab03: DAC");
    lcd_locate(0, 1);
    lcd_printf("Group: 5");
    
    while(TRUE)
    {
        uint16_t i;
        uint32_t start;
        uint16_t cmd = 0b0011011111010000;

        TOGGLEBIT(LED1_PORT);
        Nop(); 
        CLEARBIT(DAC_CS_PORT);
        Nop();

        for (i = 0; i < 16; i++)
        {
            if (cmd & 0b1000000000000000)
                SETBIT(DAC_SDI_PORT);
            else
                CLEARBIT(DAC_SDI_PORT);

            Nop();
            SETBIT(DAC_SCK_PORT);
            Nop();
            CLEARBIT(DAC_SCK_PORT);
            Nop();

            cmd <<= 1;
        }

        SETBIT(DAC_CS_PORT);
        Nop();

        CLEARBIT(DAC_LDAC_PORT);
        Nop();
        SETBIT(DAC_LDAC_PORT);

        start = interrupt_counter;
        while ((interrupt_counter - start) < 500);

//2.5V

        TOGGLEBIT(LED1_PORT);
        Nop();

        cmd = 0b0101100111000100;

        CLEARBIT(DAC_CS_PORT);
        Nop();

        for (i = 0; i < 16; i++)
        {
            if (cmd & 0b1000000000000000)
                SETBIT(DAC_SDI_PORT);
            else
                CLEARBIT(DAC_SDI_PORT);

            Nop();
            SETBIT(DAC_SCK_PORT);
            Nop();
            CLEARBIT(DAC_SCK_PORT);
            Nop();

            cmd <<= 1;
        }

        SETBIT(DAC_CS_PORT);
        Nop();

        CLEARBIT(DAC_LDAC_PORT);
        Nop();
        SETBIT(DAC_LDAC_PORT);

        start = interrupt_counter;
        while ((interrupt_counter - start) < 2000);

//3.5V

        TOGGLEBIT(LED1_PORT);
        Nop();

        cmd = 0b0101110110101100;

        CLEARBIT(DAC_CS_PORT);
        Nop();

        for (i = 0; i < 16; i++)
        {
            if (cmd & 0b1000000000000000)
                SETBIT(DAC_SDI_PORT);
            else
                CLEARBIT(DAC_SDI_PORT);

            Nop();
            SETBIT(DAC_SCK_PORT);
            Nop();
            CLEARBIT(DAC_SCK_PORT);
            Nop();

            cmd <<= 1;
        }

        SETBIT(DAC_CS_PORT);
        Nop();

        CLEARBIT(DAC_LDAC_PORT);
        Nop();
        SETBIT(DAC_LDAC_PORT);

        start = interrupt_counter;
        while ((interrupt_counter - start) < 1000);
    }
}
