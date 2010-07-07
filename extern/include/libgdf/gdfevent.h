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

#ifndef GDFEVENT_H
#define GDFEVENT_H

#include "strexception.h"

#include "user_types.h"

#include <memory>

// TODO: did i get the endianness right?
#define NEV header->num_ev[0] + 256*header->num_ev[1] + 256*256*header->num_ev[2]
#define NEV2 num_ev[0] + 256*num_ev[1] + 256*256*num_ev[2]

class GDFEventTable
{
public:
    GDFEventTable( uint8 mode, uint32 nev, float32 fs )
    {
        if( mode == 1 )
            buflen = 8+nev*6;
        else if( mode == 3 )
            buflen = 8+nev*12;
        else
            throw StrException( "Invalid Event Mode.");

        buffer = new char[buflen];
        assignMembersToBuffer( mode, nev );
        header->fsev = fs;
    }

    GDFEventTable( char *buf )
    {
        buffer = buf;
        assignBufferToMembers( );
    }

    ~GDFEventTable( )
    {
        delete[] buffer;
    }

    void assignMembersToBuffer( uint8 mode, uint32 nev );
    void assignBufferToMembers( );

    char *getBuffer( ) { return buffer; }
    size_t bufferLength( ) { return buflen; }

private:
    char *buffer;
    size_t buflen;

public:

    struct header_t
    {
        uint32 getNumEvents( ) { return NEV2; }

        uint8 mode;
        uint8 num_ev[3];
        float32 fsev;
    } *header;
    uint32 *positions;
    uint16 *type;
    uint16 *channel;
    uint32 *duration; float32 *val_noneqsamp;
};

#endif // GDFEVENT_H
