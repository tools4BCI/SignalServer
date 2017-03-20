/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Michele Tavella <michele.tavella@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    It is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this file.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TCTIME_HPP
#define TCTIME_HPP

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __MINGW32__
#undef _WIN32
#endif //__MINGW32__

#ifndef _WIN32
#include <sys/time.h>
#else
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
//struct timeval {
//	long tv_sec;
//	long tv_usec;
//};
struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

int gettimeofday (struct timeval *tv, struct timezone *tz);
//void timerclear(struct timeval *tvp);
//bool timerisset(struct timeval *tvp);
#endif //_WIN32

/*! \brief Sleep milliseconds
 *
 * \ingroup tobicore
 *
 * The implementation of this function depends on the supported plaform.
 * It calls Sleep() if compiled for MinGW, select() otherwise.
 *
 * \arg ms Amount of milliseconds to sleep for
 */
void TCSleep(double ms);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //TCTIME_HPP
