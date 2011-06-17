/* Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com */
/* MAX3421E functions */
#ifndef _MAX3421E_H_
#define _MAX3421E_H_

#if defined(__PIC32MX__)
    #include <plib.h>
    #include <WProgram.h>
    //#include <SPI.h>
    //#define byte unsigned char
    //#define boolean unsigned char
    //#define uint8_t unsigned char
#else
//#include <Spi.h>
//#include <WProgram.h>
#include "WProgram.h"
#endif
#include "Max3421e_constants.h"

class MAX3421E /* : public SPI */ {
    // byte vbusState;
    public:
        MAX3421E( void );
        byte getVbusState( void );
//        void toggle( byte pin );
        static void regWr( byte, byte );
        char * bytesWr( byte, byte, char * );
        static void gpioWr( byte );
        byte regRd( byte );
        char * bytesRd( byte, byte, char * );
        byte gpioRd( void );
        boolean reset();
        boolean vbusPwr ( boolean );
        void busprobe( void );
        void powerOn();
        byte IntHandler();
        byte GpxHandler();
        byte Task();
    private:
      static void spi_init() {
        uint8_t tmp;
        // initialize SPI pins
        pinMode(SCK_PIN, OUTPUT);
        pinMode(MOSI_PIN, OUTPUT);
        pinMode(MISO_PIN, INPUT);
        pinMode(SS_PIN, OUTPUT);
        digitalWrite( SS_PIN, HIGH ); 

#if defined(__PIC32MX__)
        // todo 1: setup spi tris registers
        // todo 1: Set SPI Mode
        // todo 3: Select proper spi bit rate
        SPI2BRG = 0x1ff; // Slow the SPI way down for my lazy scope...
#else

        /* mode 00 (CPOL=0, CPHA=0) master, fclk/2. Mode 11 (CPOL=11, CPHA=11) is also supported by MAX3421E */
        SPCR = 0x50;
        SPSR = 0x01;
        /**/
        tmp = SPSR;
        tmp = SPDR;
#endif

    }

#ifdef GONE

//        void init();
#if defined(__PIC32MX__)
      // todo: 3 impliment this
      //friend class Max_LCD;
#else
    friend class Max_LCD;        
#endif

#endif
};




#endif //_MAX3421E_H_
