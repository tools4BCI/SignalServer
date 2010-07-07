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

#ifndef GDFHEADER_H
#define GDFHEADER_H

#include "user_types.h"

#include <stdlib.h>

#define GDF_VERSION_STRING "GDF 2.10"

#if defined(_MSC_VER)
        #pragma pack (push, 1)
#elif defined(__BORLANDC__)
        #pragma option push -a1
#endif

// note: this class has data member padding disabled!

class GDFHeader
{
public:
    GDFHeader();

    void reset( );

    bool testVersion( )
    {
        char ch[] = GDF_VERSION_STRING;
        char *c1 = gdf_version_id;
        char *c2 = ch;
        for( int i=0; i<8; i++, c1++, c2++ )
            if( *c1 != *c2 )
                return false;
        return true;
    }

    char *getBuffer( ) { return (char*)this; }
    size_t bufferLength( ) { return sizeof(*this); }

public:
    char gdf_version_id[8];
    char gdf_patient_id[66];
    uint8 gdf_reserved_1[10];
    uint8 gdf_patient_drugs;
    uint8 gdf_patient_weight;
    uint8 gdf_patient_height;
    uint8 gdf_patient_flags;
    char gdf_recording_id[64];
    uint32 gdf_recording_location[4];
    uint64 gdf_recording_start;         // this is defined as uint32[2] in the spec
    uint64 gdf_patient_birthday;        // this is defined as uint32[2] in the spec
    uint16 gdf_header_length;
    char gdf_patient_ICD[6];           // this is defined as uint8 in the spec
    uint64 gdf_equipment_provider_classification;
    char gdf_reserved_2[6];
    uint16 gdf_patient_headsize[3];
    float32 gdf_pos_reference[3];
    float32 gdf_pos_ground[3];
    int64 gdf_num_datarecords;
    uint32 gdf_datarecords_duration[2];
    uint16 gdf_num_signals;
    uint16 gdf_reserved_3;
}

#ifdef __GNUC__
__attribute__((__packed__))
#endif
;

#if defined(_MSC_VER)
        #pragma pack (pop)
#elif defined(__BORLANDC__)
        #pragma option pop
#endif

#endif // GDFHEADER_H
