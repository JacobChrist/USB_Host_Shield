/* Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com */
/* USB functions */
#ifndef _usb_h_
#define _usb_h_

#include <Max3421e.h>
#include "ch9.h"
#include "core.h"

/* Common setup data constant combinations  */
#define bmREQ_GET_DESCR     USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_DEVICE     //get descriptor request type
#define bmREQ_SET           USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_DEVICE     //set request type for all but 'set feature' and 'set interface'
#define bmREQ_CL_GET_INTF   USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE     //get interface request type
/* HID requests */
#define bmREQ_HIDOUT        USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE
#define bmREQ_HIDIN         USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE 
#define bmREQ_HIDREPORT     USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_INTERFACE

#define USB_XFER_TIMEOUT    5000    //USB transfer timeout in milliseconds, per section 9.2.6.1 of USB 2.0 spec
#define USB_NAK_LIMIT       32000   //NAK limit for a transfer. o meand NAKs are not counted
#define USB_RETRY_LIMIT     3       //retry limit for a transfer
#define USB_SETTLE_DELAY    200     //settle delay in milliseconds
#define USB_NAK_NOWAIT      1       //used in Richard's PS2/Wiimote code

#define USB_NUMDEVICES  2           //number of USB devices

/* USB state machine states */

#define USB_STATE_MASK                                      0xf0

#define USB_STATE_DETACHED                                  0x10
#define USB_DETACHED_SUBSTATE_INITIALIZE                    0x11        
#define USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE               0x12
#define USB_DETACHED_SUBSTATE_ILLEGAL                       0x13
#define USB_ATTACHED_SUBSTATE_SETTLE                        0x20
#define USB_ATTACHED_SUBSTATE_RESET_DEVICE                  0x30    
#define USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE           0x40
#define USB_ATTACHED_SUBSTATE_WAIT_SOF                      0x50
#define USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE    0x60
#define USB_STATE_ADDRESSING                                0x70
#define USB_STATE_CONFIGURING                               0x80
#define USB_STATE_RUNNING                                   0x90
#define USB_STATE_ERROR                                     0xa0

// us8 usb_task_state = USB_DETACHED_SUBSTATE_INITIALIZE

/* USB Setup Packet Structure   */
typedef struct {
    union {                     // offset   description
        us8 bmRequestType;      //   0      Bit-map of request type
        struct {
            us8 recipient:  5;  //          Recipient of the request
            us8 type:       2;  //          Type of request
            us8 direction:  1;  //          Direction of data X-fer
        };
    }ReqType_u;
    us8 bRequest;               //   1      Request
    union {
        us16    wValue;         //   2      Depends on bRequest
        struct {
            us8 wValueLo;
            us8 wValueHi;
        };
    }wVal_u;
    us16    wIndex;             //   4      Depends on bRequest
    us16    wLength;            //   6      Depends on bRequest
} SETUP_PKT, *PSETUP_PKT;

/* Endpoint information structure               */
/* bToggle of endpoint 0 initialized to 0xff    */
/* during enumeration bToggle is set to 00      */
typedef struct {        
    us8  epAddr;        //copy from endpoint descriptor. Bit 7 indicates direction ( ignored for control endpoints )
    us8  Attr;          // Endpoint transfer type.
    us16 MaxPktSize;    // Maximum packet size.
    us8  Interval;      // Polling interval in frames.
    us8  sndToggle;     //last toggle value, bitmask for HCTL toggle bits
    us8  rcvToggle;     //last toggle value, bitmask for HCTL toggle bits
    /* not sure if both are necessary */
} EP_RECORD;
/* device record structure */
typedef struct {
    EP_RECORD* epinfo;      //device endpoint information
    us8  devclass;          //device class
} DEV_RECORD;



class USB : public MAX3421E {
//data structures    
/* device table. Filled during enumeration              */
/* index corresponds to device address                  */
/* each entry contains pointer to endpoint structure    */
/* and device class to use in various places            */             
//DEV_RECORD devtable[ USB_NUMDEVICES + 1 ];
//EP_RECORD dev0ep;         //Endpoint data structure used during enumeration for uninitialized device

//us8 usb_task_state;

    public:
        USB( void );
        us8 getUsbTaskState( void );
        void setUsbTaskState( us8 state );
        EP_RECORD* getDevTableEntry( us8 addr, us8 ep );
        void setDevTableEntry( us8 addr, EP_RECORD* eprecord_ptr );
        us8 ctrlReq( us8 addr, us8 ep, us8 bmReqType, us8 bRequest, us8 wValLo, us8 wValHi, us16 wInd, us16 nbytes, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
        /* Control requests */
        us8 getDevDescr( us8 addr, us8 ep, us16 nbytes, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
        us8 getConfDescr( us8 addr, us8 ep, us16 nbytes, us8 conf, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
        us8 getStrDescr( us8 addr, us8 ep, us16 nbytes, us8 index, us16 langid, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
        us8 setAddr( us8 oldaddr, us8 ep, us8 newaddr, us16 nak_limit = USB_NAK_LIMIT );
        us8 setConf( us8 addr, us8 ep, us8 conf_value, us16 nak_limit = USB_NAK_LIMIT );
        /**/
        us8 setProto( us8 addr, us8 ep, us8 interface, us8 protocol, us16 nak_limit = USB_NAK_LIMIT );
        us8 getProto( us8 addr, us8 ep, us8 interface, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
        us8 getReportDescr( us8 addr, us8 ep, us16 nbytes, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
        us8 setReport( us8 addr, us8 ep, us16 nbytes, us8 interface, us8 report_type, us8 report_id, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
              us8 getReport( us8 addr, us8 ep, us16 nbytes, us8 interface, us8 report_type, us8 report_id, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
        us8 getIdle( us8 addr, us8 ep, us8 interface, us8 reportID, char* dataptr, us16 nak_limit = USB_NAK_LIMIT );
        us8 setIdle( us8 addr, us8 ep, us8 interface, us8 reportID, us8 duration, us16 nak_limit = USB_NAK_LIMIT );
        /**/
        us8 ctrlData( us8 addr, us8 ep, us16 nbytes, char* dataptr, boolean direction, us16 nak_limit = USB_NAK_LIMIT );
        us8 ctrlStatus( us8 ep, boolean direction, us16 nak_limit = USB_NAK_LIMIT );
        us8 inTransfer( us8 addr, us8 ep, us16 nbytes, char* data, us16 nak_limit = USB_NAK_LIMIT );
        us8 outTransfer( us8 addr, us8 ep, us16 nbytes, char* data, us16 nak_limit = USB_NAK_LIMIT );
        us8 dispatchPkt( us8 token, us8 ep, us16 nak_limit = USB_NAK_LIMIT );
        void Task( void );
    private:
        void init();
};

//get device descriptor
inline us8 USB::getDevDescr( us8 addr, us8 ep, us16 nbytes, char* dataptr, us16 nak_limit ) {
    return( ctrlReq( addr, ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, 0x00, USB_DESCRIPTOR_DEVICE, 0x0000, nbytes, dataptr, nak_limit ));
}
//get configuration descriptor  
inline us8 USB::getConfDescr( us8 addr, us8 ep, us16 nbytes, us8 conf, char* dataptr, us16 nak_limit ) {
        return( ctrlReq( addr, ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, conf, USB_DESCRIPTOR_CONFIGURATION, 0x0000, nbytes, dataptr, nak_limit ));
}
//get string descriptor
inline us8 USB::getStrDescr( us8 addr, us8 ep, us16 nbytes, us8 index, us16 langid, char* dataptr, us16 nak_limit ) {
    return( ctrlReq( addr, ep, bmREQ_GET_DESCR, USB_REQUEST_GET_DESCRIPTOR, index, USB_DESCRIPTOR_STRING, langid, nbytes, dataptr, nak_limit ));
}
//set address 
inline us8 USB::setAddr( us8 oldaddr, us8 ep, us8 newaddr, us16 nak_limit ) {
    return( ctrlReq( oldaddr, ep, bmREQ_SET, USB_REQUEST_SET_ADDRESS, newaddr, 0x00, 0x0000, 0x0000, NULL, nak_limit ));
}
//set configuration
inline us8 USB::setConf( us8 addr, us8 ep, us8 conf_value, us16 nak_limit ) {
    return( ctrlReq( addr, ep, bmREQ_SET, USB_REQUEST_SET_CONFIGURATION, conf_value, 0x00, 0x0000, 0x0000, NULL, nak_limit ));         
}
//class requests
inline us8 USB::setProto( us8 addr, us8 ep, us8 interface, us8 protocol, us16 nak_limit ) {
        return( ctrlReq( addr, ep, bmREQ_HIDOUT, HID_REQUEST_SET_PROTOCOL, protocol, 0x00, interface, 0x0000, NULL, nak_limit ));
}
inline us8 USB::getProto( us8 addr, us8 ep, us8 interface, char* dataptr, us16 nak_limit ) {
        return( ctrlReq( addr, ep, bmREQ_HIDIN, HID_REQUEST_GET_PROTOCOL, 0x00, 0x00, interface, 0x0001, dataptr, nak_limit ));        
}
//get HID report descriptor 
inline us8 USB::getReportDescr( us8 addr, us8 ep, us16 nbytes, char* dataptr, us16 nak_limit ) {
        return( ctrlReq( addr, ep, bmREQ_HIDREPORT, USB_REQUEST_GET_DESCRIPTOR, 0x00, HID_DESCRIPTOR_REPORT, 0x0000, nbytes, dataptr, nak_limit ));
}
inline us8 USB::setReport( us8 addr, us8 ep, us16 nbytes, us8 interface, us8 report_type, us8 report_id, char* dataptr, us16 nak_limit ) {
    return( ctrlReq( addr, ep, bmREQ_HIDOUT, HID_REQUEST_SET_REPORT, report_id, report_type, interface, nbytes, dataptr, nak_limit ));
}
inline us8 USB::getReport( us8 addr, us8 ep, us16 nbytes, us8 interface, us8 report_type, us8 report_id, char* dataptr, us16 nak_limit ) { // ** RI 04/11/09
    return( ctrlReq( addr, ep, bmREQ_HIDIN, HID_REQUEST_GET_REPORT, report_id, report_type, interface, nbytes, dataptr, nak_limit ));
}
/* returns one us8 of data in dataptr */
inline us8 USB::getIdle( us8 addr, us8 ep, us8 interface, us8 reportID, char* dataptr, us16 nak_limit ) {
        return( ctrlReq( addr, ep, bmREQ_HIDIN, HID_REQUEST_GET_IDLE, reportID, 0, interface, 0x0001, dataptr, nak_limit ));    
}
inline us8 USB::setIdle( us8 addr, us8 ep, us8 interface, us8 reportID, us8 duration, us16 nak_limit ) {
           return( ctrlReq( addr, ep, bmREQ_HIDOUT, HID_REQUEST_SET_IDLE, reportID, duration, interface, 0x0000, NULL, nak_limit ));
          }
#endif //_usb_h_
