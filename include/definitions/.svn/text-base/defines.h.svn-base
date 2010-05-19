
/**
* @file defines.h
*
* @brief defines is used to store preprocessor macros.
*
* In defines preprocessor(PPM) macros are defined.
* This macros are not intended to be changed, but can be extended if needed.
* PPM have to be used instead of hard coded values in the code.
*
**/

#ifndef DEFINES_H
#define DEFINES_H

//-----------------------------------------------------------------------------
/*! \addtogroup hardware_device_indentifiers
*  Every hardware device is is identified by it's own identifier (unsigned int).
*  Those identifiers have to be declared here.
*  If new supported hardware is included into the signalserver, this section has to be
*  extended. Just increase the running number by 1. (an identifier must not occur multiple times!)
*  @{
*/

#define SINEGEN   1
#define USBAMP    2
#define MOBILAB   3
#define JOYSTICK  4

/*! @} */


/*! \addtogroup data_packet_definitions
*  Definitions concerning the DataPacket have to be done here!
*  This section is especially built for flags, used in the DataPacket.
*  The first 3 bytes are planned to be used for different signal flags,
*  whereby the last flag means an undefined signal type, the 4 bits before
*  "undefined" are user defined signals.
*  The last 6 bits are used for the packet version, if the packet undergoes different adoptions
*  during time.
*  The use of 2 bits is up to now not fixed!
*  @{
*/

//  4000000   ... first packetversion
//  8000000   ... timestamp added
#define PACKETVERSION 0x8000000   // --> 2^27  .... first PacketVersion Bit

#define PACKETCODE    0x2400000   // Bits 23 and 26  are 1
#define BITS4PACKETVERSION 6
#define BYTES4SIGNALFLAGS 3

#define SIG_EEG 0x01
#define SIG_EMG 0x02
#define SIG_EOG 0x04
#define SIG_ECG 0x08
#define SIG_HR 0x10
#define SIG_BP 0x20
#define SIG_BUTTON 0x40
#define SIG_JOYSTICK 0x80
#define SIG_SENSOR 0x100

#define SIG_NIRS 0x200
#define SIG_FMRI 0x400

#define SIG_USER_1    0x10000
#define SIG_USER_2    0x20000
#define SIG_USER_3    0x40000
#define SIG_USER_4    0x80000
#define SIG_UNDEFINED 0x100000
#define SIG_EVENT     0x200000

/*! @} */


/*! \addtogroup miscellaneous
*
*  @{
*/

#define PI 3.141592653589793238
#define BUFFER_SIZE 8388608     //in bytes
#define EVENT_BUFFER_SIZE  8192

/*! @} */
//-----------------------------------------------------------------------------

#endif // DEFINES_H

