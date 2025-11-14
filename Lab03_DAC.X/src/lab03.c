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

//SETS 1 = input pin , 0 = output pin
#define DAC_CS_TRIS TRISDbits.TRISD8
#define DAC_SDI_TRIS TRISBbits.TRISB10
#define DAC_SCK_TRIS TRISBbits.TRISB11
#define DAC_LDAC_TRIS TRISBbits.TRISB13
    
//READS THE ACTUAL VALUE
#define DAC_CS_PORT PORTDbits.RD8
#define DAC_SDI_PORT PORTBbits.RB10
#define DAC_SCK_PORT PORTBbits.RB11
#define DAC_LDAC_PORT PORTBbits.RB13

//SETS 1 = DIGITAL PIN, 0 = ANALOG PIN
#define DAC_SDI_AD1CFG AD1PCFGLbits.PCFG10
#define DAC_SCK_AD1CFG AD1PCFGLbits.PCFG11
#define DAC_LDAC_AD1CFG AD1PCFGLbits.PCFG13

#define DAC_SDI_AD2CFG AD2PCFGLbits.PCFG10
#define DAC_SCK_AD2CFG AD2PCFGLbits.PCFG11
#define DAC_LDAC_AD2CFG AD2PCFGLbits.PCFG13

void dac_initialize()
{   
    // set AN10, AN11 AN13 to digital mode
    SETBIT(AD1PCFGLbits.PCFG10);
    SETBIT(AD1PCFGLbits.PCFG11);
    SETBIT(AD1PCFGLbits.PCFG13);
    
    // this means AN10 will become RB10, AN11->RB11, AN13->RB13
    // see datasheet 11.3
    
    // set RD8, RB10, RB11, RB13 as output pins
    CLEARBIT(TRISDbits.TRISD8);
    CLEARBIT(TRISBbits.TRISB10);
    CLEARBIT(TRISBbits.TRISB11);
    CLEARBIT(TRISBbits.TRISB13);
    
    // set default state: CS=??, SCK=??, SDI=??, LDAC=??
    SETBIT(PORTDbits.RD8); //NOT CS = 1 - default idle
    CLEARBIT(PORTBbits.RB10); // SCK = default low
    CLEARBIT(PORTBbits.RB11); // SDI = default low
    SETBIT(PORTBbits.RB13); // NOT LDAC = 1 no updates
    
}

/*
 * Timer code
 */

#define FCY_EXT   32768UL

#define TCKPS_1   0x00
#define TCKPS_8   0x01
#define TCKPS_64  0x02
#define TCKPS_256 0x03

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
    
}

// interrupt service routine?

/*
 * main loop
 */

void main_loop()
{
    uint8_t i = 0;
    uint16_t cmd = 0x37D0;
    
    // print assignment information
    lcd_printf("Lab03: DAC");
    lcd_locate(0, 1);
    lcd_printf("Group: SP5");
    
    while(TRUE)
    {
        CLEARBIT(PORTDbits.RD8);
        for(i = 0; i < 16; i++)
        {
            CLEARBIT(PORTBbits.RB11); //SCK to low
            
            if (cmd&0x8000) //check MSB if its 1
                SETBIT(PORTBbits.RB10); // SDI to data
            else
                CLEARBIT(PORTBbits.RB10); // SDI to data
            
            SETBIT(PORTBbits.RB11); // SCK to high
            Nop();            
            
            cmd <<= 1; // shift cmd value to the left, to check next bit
        }
        SETBIT(PORTDbits.RD8);
        
        CLEARBIT(PORTBbits.RB13);
        Nop();
        SETBIT(PORTBbits.RB13);
        Nop();
    }
}
