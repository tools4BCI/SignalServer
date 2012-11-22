/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

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

#include <boost/cstdint.hpp>

typedef boost::uint32_t SignalTypeFlag;

SignalTypeFlag const SIG_EEG        = 0x1;
SignalTypeFlag const SIG_EMG        = 0x1 << 1;
SignalTypeFlag const SIG_EOG        = 0x1 << 2;
SignalTypeFlag const SIG_ECG        = 0x1 << 3;
SignalTypeFlag const SIG_HR         = 0x1 << 4;
SignalTypeFlag const SIG_BP         = 0x1 << 5;
SignalTypeFlag const SIG_BUTTON     = 0x1 << 6;
SignalTypeFlag const SIG_AXIS       = 0x1 << 7;
SignalTypeFlag const SIG_SENSOR     = 0x1 << 8;
SignalTypeFlag const SIG_NIRS       = 0x1 << 9;
SignalTypeFlag const SIG_FMRI       = 0x1 << 10;
SignalTypeFlag const SIG_KEYCODE    = 0x1 << 11;

SignalTypeFlag const SIG_USER_1     = 0x1 << 16;
SignalTypeFlag const SIG_USER_2     = 0x1 << 17;
SignalTypeFlag const SIG_USER_3     = 0x1 << 18;
SignalTypeFlag const SIG_USER_4     = 0x1 << 19;
SignalTypeFlag const SIG_UNDEFINED  = 0x1 << 20;
SignalTypeFlag const SIG_EVENT      = 0x1 << 21;

/*! @} */


/*! \addtogroup miscellaneous
*
*  @{
*/

//#define PI 3.141592653589793238
#define BUFFER_SIZE 8388608     //in bytes
#define EVENT_BUFFER_SIZE  8192

/*! @} */
//-----------------------------------------------------------------------------

#endif // DEFINES_H

