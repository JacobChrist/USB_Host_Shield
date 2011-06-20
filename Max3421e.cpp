/* Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com */
/* MAX3421E USB host controller support */

#include "Max3421e.h"
#include "Max3421e_constants.h"

static us8 vbusState;


/* Constructor */
MAX3421E::MAX3421E()
{
    Serial.println("MAX3421E()");
    pinMode( MAX_INT, INPUT);
    pinMode( MAX_GPX, INPUT );
    pinMode( MAX_SS, OUTPUT );
    digitalWrite(MAX_SS,HIGH);   
    pinMode( MAX_RESET, OUTPUT );
    digitalWrite( MAX_RESET, HIGH );  //release MAX3421E from reset
#if defined(__PIC32MX__)
    spi_init();
#else
#endif
}

/* Functions    */
us8 MAX3421E::spi_swap(us8 _data) {
    while( ((1 << bnTbe) & SPI2STAT) == 0 );
    SPI2BUF = _data;
    while( ((1 << bnRbf) & SPI2STAT) == 0 );
    return SPI2BUF;
}
#if defined(__PIC32MX__)
#else
#endif

us8 MAX3421E::getVbusState( void )
{ 
    return( vbusState );
}
/* initialization */
//void MAX3421E::init()
//{
//    /* setup pins */
//    pinMode( MAX_INT, INPUT);
//    pinMode( MAX_GPX, INPUT );
//    pinMode( MAX_SS, OUTPUT );
//    //pinMode( BPNT_0, OUTPUT );
//    //pinMode( BPNT_1, OUTPUT );
//    //digitalWrite( BPNT_0, LOW );
//    //digitalWrite( BPNT_1, LOW );
//    Deselect_MAX3421E;              
//    pinMode( MAX_RESET, OUTPUT );
//    digitalWrite( MAX_RESET, HIGH );  //release MAX3421E from reset
//}
//us8 MAX3421E::getVbusState( void )
//{
//    return( vbusState );
//}
//void MAX3421E::toggle( us8 pin )
//{
//    digitalWrite( pin, HIGH );
//    digitalWrite( pin, LOW );
//}
/* Single host register write   */
void MAX3421E::regWr( us8 reg, us8 val)
{
    us8 tmp;
/*
    Serial.print("regWr(");
    Serial.print(reg, HEX);
    Serial.print(")=");
    Serial.print(val, HEX);
    Serial.print("");
*/
#if defined(__PIC32MX__)
    digitalWrite( MAX_SS,LOW );
    tmp = spi_swap( reg | 0x02 );
    tmp = spi_swap( val );
    digitalWrite( MAX_SS,HIGH );
#else
      digitalWrite(MAX_SS,LOW);
      SPDR = ( reg | 0x02 );
      while(!( SPSR & ( 1 << SPIF )));
      SPDR = val;
      while(!( SPSR & ( 1 << SPIF )));
      digitalWrite(MAX_SS,HIGH);
#endif
     // Serial.println("*");
      return;
}
/* multiple-byte write */
/* returns a pointer to a memory position after last written */
char * MAX3421E::bytesWr( us8 reg, us8 nbytes, char * data )
{
    us8 tmp;
#if defined(__PIC32MX__)
    digitalWrite( MAX_SS, LOW );
    tmp = spi_swap( reg | 0x02 );
    while( nbytes-- ) {               // send next data byte
      tmp = spi_swap( *data );
      data++;                         // advance data pointer
    }
    digitalWrite( MAX_SS, HIGH );
#else
    digitalWrite(MAX_SS,LOW);
    SPDR = ( reg | 0x02 );
    while( nbytes-- ) {
      while(!( SPSR & ( 1 << SPIF )));  //check if previous byte was sent
      SPDR = ( *data );               // send next data byte
      data++;                         // advance data pointer
    }
    while(!( SPSR & ( 1 << SPIF )));
    digitalWrite(MAX_SS,HIGH);
#endif
    return( data );
}
/* GPIO write. GPIO byte is split between 2 registers, so two writes are needed to write one byte */
/* GPOUT bits are in the low nibble. 0-3 in IOPINS1, 4-7 in IOPINS2 */
/* upper 4 bits of IOPINS1, IOPINS2 are read-only, so no masking is necessary */
void MAX3421E::gpioWr( us8 val )
{
    regWr( rIOPINS1, val );
    val = val >>4;
    regWr( rIOPINS2, val );
    
    return;     
}
/* Single host register read        */
us8 MAX3421E::regRd( us8 reg )
{
    us8 tmp;
    us8 status;
#if defined(__PIC32MX__)
  digitalWrite(MAX_SS,LOW);
  status = spi_swap( reg );
  tmp = spi_swap( 0 );
  digitalWrite(MAX_SS,HIGH);
  return( tmp );
#else
    digitalWrite(MAX_SS,LOW);
    SPDR = reg;
    while(!( SPSR & ( 1 << SPIF )));
    SPDR = 0; //send empty byte
    while(!( SPSR & ( 1 << SPIF )));
    digitalWrite(MAX_SS,HIGH); 
    return( SPDR );
#endif
}
/* multiple-bytes register read                             */
/* returns a pointer to a memory position after last read   */
char * MAX3421E::bytesRd ( us8 reg, us8 nbytes, char  * data )
{
    us8 tmp;
#if defined(__PIC32MX__)
    digitalWrite(MAX_SS,LOW);
    tmp = spi_swap( reg );
    while( nbytes ) {
      *data = spi_swap( 0 );
      nbytes--;
      data++;
    }
    digitalWrite(MAX_SS,HIGH);
#else
    digitalWrite(MAX_SS,LOW);
    SPDR = reg;      
    while(!( SPSR & ( 1 << SPIF )));    //wait
    while( nbytes ) {
      SPDR = 0; //send empty byte
      nbytes--;
      while(!( SPSR & ( 1 << SPIF )));
      *data = SPDR;
      data++;
    }
    digitalWrite(MAX_SS,HIGH);
#endif
    return( data );
}
/* GPIO read. See gpioWr for explanation */
/* GPIN pins are in high nibbles of IOPINS1, IOPINS2    */
us8 MAX3421E::gpioRd( void )
{
 us8 tmpbyte = 0;
    tmpbyte = regRd( rIOPINS2 );            //pins 4-7
    tmpbyte &= 0xf0;                        //clean lower nibble
    tmpbyte |= ( regRd( rIOPINS1 ) >>4 ) ;  //shift low bits and OR with upper from previous operation. Upper nibble zeroes during shift, at least with this compiler
    return( tmpbyte );
}
/* reset MAX3421E using chip reset bit. SPI configuration is not affected   */
boolean MAX3421E::reset()
{
  us8 tmp = 0;
  us8 result = 0xff;
    // Note, reset does not take SPI out of full duplex (page 6 of the programing guide)
    regWr( rUSBCTL, bmCHIPRES );                        //Chip reset. This stops the oscillator
    regWr( rUSBCTL, 0x00 );                             //Remove the reset
    do {
        delay(100);
        result = regRd( rUSBIRQ );
        Serial.print("rUSBIRQ=0x");
        Serial.println(result,HEX);

        tmp++;                                          //timeout after 64k attempts
        if( tmp == 0x00 ) {
            return( false );
        }
    } while(!(result & bmOSCOKIRQ ));          //wait until the PLL is stable
    return( true );
}
/* turn USB power on/off                                                */
/* does nothing, returns TRUE. Left for compatibility with old sketches               */
/* will be deleted eventually                                           */
///* ON pin of VBUS switch (MAX4793 or similar) is connected to GPOUT7    */
///* OVERLOAD pin of Vbus switch is connected to GPIN7                    */
///* OVERLOAD state low. NO OVERLOAD or VBUS OFF state high.              */
boolean MAX3421E::vbusPwr ( boolean action )
{
//  us8 tmp;
//    tmp = regRd( rIOPINS2 );                //copy of IOPINS2
//    if( action ) {                          //turn on by setting GPOUT7
//        tmp |= bmGPOUT7;
//    }
//    else {                                  //turn off by clearing GPOUT7
//        tmp &= ~bmGPOUT7;
//    }
//    regWr( rIOPINS2, tmp );                 //send GPOUT7
//    if( action ) {
//        delay( 60 );
//    }
//    if (( regRd( rIOPINS2 ) & bmGPIN7 ) == 0 ) {     // check if overload is present. MAX4793 /FLAG ( pin 4 ) goes low if overload
//        return( false );
//    }                      
    return( true );                                             // power on/off successful                       
}
/* probe bus to determine device presense and speed and switch host to this speed */
void MAX3421E::busprobe( void )
{
 us8 bus_sample;
    bus_sample = regRd( rHRSL );            //Get J,K status
    bus_sample &= ( bmJSTATUS|bmKSTATUS );      //zero the rest of the byte
    switch( bus_sample ) {                          //start full-speed or low-speed host 
        case( bmJSTATUS ):
            if(( regRd( rMODE ) & bmLOWSPEED ) == 0 ) {
                regWr( rMODE, MODE_FS_HOST );       //start full-speed host
                vbusState = FSHOST;
            }
            else {
                regWr( rMODE, MODE_LS_HOST);        //start low-speed host
                vbusState = LSHOST;
            }
            break;
        case( bmKSTATUS ):
            if(( regRd( rMODE ) & bmLOWSPEED ) == 0 ) {
                regWr( rMODE, MODE_LS_HOST );       //start low-speed host
                vbusState = LSHOST;
            }
            else {
                regWr( rMODE, MODE_FS_HOST );       //start full-speed host
                vbusState = FSHOST;
            }
            break;
        case( bmSE1 ):              //illegal state
            vbusState = SE1;
            break;
        case( bmSE0 ):              //disconnected state
            vbusState = SE0;
            break;
        }//end switch( bus_sample )
}
void MAX3421E::reg_dump(void){
    us8 reg;
    for( reg = 0; reg <=31; reg++ ) {
        Serial.print("reg(");
        Serial.print(reg, DEC);
        Serial.print(") = 0x");
        Serial.println(regRd( reg ), HEX);
    }
    delay(10000);
}

/* MAX3421E initialization after power-on   */
void MAX3421E::powerOn()
{
    us8 result;
    /* Configure full-duplex SPI, interrupt pulse   */
    Serial.println("");
    Serial.println("Starting power on sequence...");
    regWr( rPINCTL,( bmFDUPSPI | bmINTLEVEL | bmGPXB ));    //Full-duplex SPI, level interrupt, GPX
    Serial.print("rPINCTL = 0x");
    Serial.println( regRd( rPINCTL ), HEX);
    if( reset() == false ) {                                //stop/start the oscillator
        Serial.println("Error: OSCOKIRQ failed to assert");
    }
    else {
        Serial.println("Status: OSCOKIRQ asserted");
    }

    /* configure host operation */
    regWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ );      // set pull-downs, Host, Separate GPIN IRQ on GPX
    regWr( rHIEN, bmCONDETIE|bmFRAMEIE );                                             //connection detection
    /* check if device is connected */

    regWr( rHCTL,bmSAMPLEBUS );                                             // sample USB bus
    do {
        //delay(250);
        result = regRd( rHCTL );
        Serial.print("rHCTL=0x");
        Serial.println(result,HEX);
    } while(!(result  & bmSAMPLEBUS ));                                //wait for sample operation to finish
    busprobe();                                                             //check if anything is connected
    regWr( rHIRQ, bmCONDETIRQ );                                            //clear connection detect interrupt                 
    regWr( rCPUCTL, 0x01 );                                                 //enable interrupt pin
}
/* MAX3421 state change task and interrupt handler */
us8 MAX3421E::Task( void )
{
 us8 rcode = 0;
 us8 pinvalue;
    //Serial.print("Vbus state: ");
    //Serial.println( vbusState, HEX );
    pinvalue = digitalRead( MAX_INT );    
    if( pinvalue  == LOW ) {
        rcode = IntHandler();
    }
    pinvalue = digitalRead( MAX_GPX );
    if( pinvalue == LOW ) {
        GpxHandler();
    }
//    usbSM();                                //USB state machine                            
    return( rcode );   
}   
us8 MAX3421E::IntHandler()
{
 us8 HIRQ;
 us8 HIRQ_sendback = 0x00;
    HIRQ = regRd( rHIRQ );                  //determine interrupt source
    //if( HIRQ & bmFRAMEIRQ ) {               //->1ms SOF interrupt handler
    //    HIRQ_sendback |= bmFRAMEIRQ;
    //}//end FRAMEIRQ handling
    if( HIRQ & bmCONDETIRQ ) {
        busprobe();
        HIRQ_sendback |= bmCONDETIRQ;
    }
    /* End HIRQ interrupts handling, clear serviced IRQs    */
    regWr( rHIRQ, HIRQ_sendback );
    return( HIRQ_sendback );
}
us8 MAX3421E::GpxHandler()
{
 us8 GPINIRQ = regRd( rGPINIRQ );          //read GPIN IRQ register
//    if( GPINIRQ & bmGPINIRQ7 ) {            //vbus overload
//        vbusPwr( OFF );                     //attempt powercycle
//        delay( 1000 );
//        vbusPwr( ON );
//        regWr( rGPINIRQ, bmGPINIRQ7 );
//    }       
    return( GPINIRQ );
}

//void MAX3421E::usbSM( void )                //USB state machine
//{
//    
//
//}
