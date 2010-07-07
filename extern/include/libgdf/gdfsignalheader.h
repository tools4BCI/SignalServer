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

#ifndef GDFSIGNALHEADER_H
#define GDFSIGNALHEADER_H

#include "user_types.h"

#include <stdlib.h>

class GDFSignalHeader
{
public:
    GDFSignalHeader( uint16 NS );
    ~GDFSignalHeader( );

    GDFSignalHeader( const GDFSignalHeader &other );    // copy constructor
    GDFSignalHeader &operator=( const GDFSignalHeader &other );

    void assignBuffer( );

    void reset( );

    char *getBuffer( ) { return buffer; }
    size_t bufferLength( ) { return 256*NS_; }


    bool isSparse( uint16 chan );

public:

    typedef char label_t[16];
    typedef char transtyp_t[80];
    typedef char physdim_t[6];       // the spec defines this is a 6 byte long char[8] (but it's obsolete anyway)
    typedef float32 senspos_t[3];
    typedef uint8 sensinf_t[20];
    typedef char reserved_t[32];

    label_t *label;
    transtyp_t *transducer_type;
    physdim_t *physical_dimension;   // obsolete
    uint16 *physical_dimension_code;
    float64 *physmin, *physmax;
    float64 *digmin, *digmax;
    float32 *lowpass;
    float32 *highpass;
    float32 *notch;
    uint32 *samples_per_record; // 0 means non-equidistant sampling
    uint32 *datatype;
    senspos_t *sensor_pos;     // the specification is not clear if this should be stored XYZXYZXYZ... or XXX..YYY...ZZZ... ?
    sensinf_t *sensor_info;

private:
    char *buffer;
    uint16 NS_;

};

#endif // GDFSIGNALHEADER_H
