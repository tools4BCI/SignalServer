/*
	Copyright (C) 2011 Francesco Leotta <francescoleotta@hotmail.com>
	Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
	Michele Tavella <michele.tavella@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TCTIME_HPP
#define TCTIME_HPP

#ifndef _WIN32
#include <sys/time.h>
#else
#include <winsock.h>

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
int gettimeofday (struct timeval *tv, struct timezone *tz);

#endif //_WIN32

#endif //TOBI_TIME_H
