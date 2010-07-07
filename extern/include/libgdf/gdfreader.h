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

#ifndef GDFREADER_H_INCLUDED
#define GDFREADER_H_INCLUDED

#include "gdfheader.h"
#include "gdfsignalheader.h"
#include "gdftagheader.h"
#include "gdfdata.h"
#include "gdfevent.h"

#include <vector>
#include <map>
#include <fstream>
#include <string>

class GDFReader
{
public:
    GDFReader( std::string fname = "" );
    ~GDFReader( );
    void clear( );

    uint16 getNumSignals( );

    void open( std::string fname );

    void close( );

    GDFData::Record *readNextRecord( );
    double getSample( int16 chan, size_t n );
    template<class T>  T getRawSample( int16 chan, size_t n )
    {
        std::map<uint16,uint16>::iterator channel = channel_mapping.find( chan );
        if( channel == channel_mapping.end( ) )
            throw StrException( "Attempt to read sample from sparse channel.");

        return record->channels[channel->second]->getRawSample<T>( n );
    }

    GDFEventTable *readEventTable( );

    GDFHeader *getHeader( );
    GDFSignalHeader *getSignalHeader( );
    GDFTagHeader *getTagHeader( );
    GDFEventTable *getEventTable( );

private:
    std::map<uint16,uint16> channel_mapping;

    std::ifstream file;

    uint32 rec_dur_num, rec_dur_den;

    GDFHeader *header;
    GDFSignalHeader *signal_header;
    GDFTagHeader *tag_header;

    GDFData::Record *record;
    int64 records_read;

    GDFEventTable *events;
};

#endif // GDFWRITER_H
