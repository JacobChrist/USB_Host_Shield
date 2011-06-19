/* Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com */
/* MAX3421E functions */
#ifndef _MAX3421E_H_
#define _MAX3421E_H_

#if defined(__PIC32MX__)
/*  Hardware Connections Needed to Connect chipKIT Uno32 to USB Host Shield
    Signal  Uno32   USB Shld
    ======= ======= =======
    +5V     5.0V    VBUS
    +3.3V   3.3V    3.3V
    GND     GND     GND
    SCLK    SCLK2   SCLK
    MOSI    SDO2    MOSI
    MISO    SDI2    MISO
    CS              SS
    INT             INT
    GPX             GPX
    RESET           RESET

  #define SCK_PIN   13
  #define MISO_PIN  12
  #define MOSI_PIN  11
  #define SS_PIN    10

*/

    #include <plib.h>
    #include <WProgram.h>
    //#include <SPI.h>

/*	SPIxCON
*/
#define bnOn	15
#define bnSmp	9
#define bnCke	8
#define bnCkp	6
#define bnMsten 5

/*	SPIxSTAT
*/
#define bnTbe	3
#define bnRbf	0

/*	IEC0
*/
#define bnSPI2RXIE	7
#define bnSPI2TXIE	6

/* SPI Connection
*/#define trisSS				TRISG
#define latSS				LATG
#define	bnSS				9

#define	trisSDO				TRISG
#define	latSDO				LATG
#define	bnSDO				8

#define trisSDI				TRISG
#define	latSDI				LATG
#define bnSDI				7

#define trisSCK				TRISG
#define latSCK				LATG
#define bnSCK				6
/********************************/

#define SPI_CLOCK_DIV4 0x01
#define SPI_CLOCK_DIV16 0x7
#define SPI_CLOCK_DIV64 0x1F
#define SPI_CLOCK_DIV128 0x3F
#define SPI_CLOCK_DIV2 0x00
#define SPI_CLOCK_DIV8 0x03
#define SPI_CLOCK_DIV32 0x0F

#define SPI_MODE0 0x00  // CKP = 0 CKE = 0
#define SPI_MODE1 0x100 // CKP = 0 CKE = 1
#define SPI_MODE2 0x40  // CKP = 1 CKE = 0
#define SPI_MODE3 0x140 // CKP = 1 CKE = 1

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
        void regWr( byte, byte );
        char * bytesWr( byte, byte, char * );
        void gpioWr( byte );
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
        byte spi_swap(byte _data);
        void reg_dump(void);
#if defined(__PIC32MX__)
#else
#endif
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
        /* SPI Header
         * SS  P1 Output
         * SDO P2 Output
         * SDI P3 Input
         * SCK P4 Output
         */
        trisSS &= ~(1 << bnSS);
        trisSDO &= ~(1 << bnSDO);
        trisSDI |= (1 << bnSDI);
        trisSCK &= ~(1 << bnSCK);

        /// Warning: if the SS pin ever becomes a LOW INPUT then SPI
        // automatically switches to Slave, so the data direction of
        // the SS pin MUST be kept as OUTPUT.
        SPI2CON = ( 1 << bnOn) | ( 1 << bnMsten ) | SPI_MODE1;
        // todo 3: Select proper spi bit rate
        //SPI2BRG = 0x1ff; // Slow the SPI way down for my lazy scope...
        SPI2BRG = 0x2; // Full speed SPI (pushing the limits of the MAX)
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
