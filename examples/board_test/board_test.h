/* USB Host Shield board test sketch header */
#ifndef _BOARD_TEST_H_
#define _BOARD_TEST_H_

/* PGMSPACE */
#include <inttypes.h>
#if defined(__PIC32MX__)
    // neither PROGMEM or PSTR are needed for PIC32, just define them as null
    #define PROGMEM
    #define PSTR(s) (s)
#else
#include <avr/pgmspace.h>
#endif

/* Messages */
const char startBanner  [] PROGMEM = "\r\nCircuits At Home 2010"
                                      "\r\nUSB Host Shield QC test routine\r\n";
const char anykey_msg [] PROGMEM =  "\r\nPress any key to continue...";
const char testpassed_msg [] PROGMEM = "\r\nTest PASSED";
const char testfailed_msg [] PROGMEM = "\r\nTest FAILED*!*";
const char osctest_oscstate_msg [] PROGMEM = " Oscillator state is ";
const char test_halted_msg [] PROGMEM = "\r\nTest Halted."
                                        "\r\n0x55 pattern is being transmitted via SPI to aid in troubleshooting";
const char spitest_fail_msg [] PROGMEM = "\r\nSPI transmit/receive mismatch"
                                          "\r\nValue written: ";

#endif
