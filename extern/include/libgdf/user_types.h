//
// This file is part of libGDF.
//
// libGDF is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// libGDF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libGDF.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2010 Martin Billinger

#ifndef USER_TYPES_H
#define USER_TYPES_H

// operating system dependent
extern const char* DIR_SEPARATOR;
extern const char* HOME_DIRECTORY;

// standard types
typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
/*typedef long long int64;
typedef unsigned long long uint64;*/
typedef float float32;
typedef double float64;

// dummy types
typedef struct { char bytes[3]; } int24;
typedef struct { char bytes[3]; } uint24;
typedef struct { char bytes[16]; } float128;

#define UINT24_MAX 16777215

// compiler specific types
#ifdef _MSC_VER
typedef __int64 int64;
typedef unsigned __int64 uint64;
#endif
#ifdef __GNUG__
typedef long long int64;
typedef unsigned long long uint64;
#endif

#ifdef _BIG_ENDIAN
// change byte order
extern inline void swapBytes(int8 &);
extern inline void swapBytes(uint8 &);
extern inline void swapBytes(int16 &value);
extern inline void swapBytes(uint16 &value);
extern inline void swapBytes(int32 &value);
extern inline void swapBytes(uint32 &value);
extern inline void swapBytes(int64 &value);
extern inline void swapBytes(uint64 &value);
extern inline void swapBytes(float32 &value);
extern inline void swapBytes(float64 &value);
#endif

#endif // USER_TYPES_H
