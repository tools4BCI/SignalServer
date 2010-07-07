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

#ifndef GDFWRITER_H
#define GDFWRITER_H

#include "gdfheader.h"
#include "gdfsignalheader.h"
#include "gdftagheader.h"
#include "gdfdata.h"

#include <vector>
#include <map>
#include <fstream>
#include <string>

class GDFWriter
{
public:
    GDFWriter( std::string file );
    ~GDFWriter( );

    // resets the state of the instance to that of a newly created class.
    void clear( );

    // add num signals of specified datatype and samplerate.
    // this is used to determine file header size and record size
    uint16 addSignal( uint32 datatype, uint32 samplerate, uint16 num = 1 );

    // adds signals with non-equidistant (sparse) sampling
    uint16 addSparseSignal( uint32 datatype, uint16 num = 1 );

    size_t getNumSignals( ) { return signal_types.size( ); }

    void forceRecordDuration( bool f, uint32 num = 1, uint32 den = 1 );

    void addTLV( uint8 tag, uint32 length, const void *val );

    // generates File Headers and sets all attributes that are known from the signals added so far.
    // this should initialize the headers with usable default values, ready for recording.
    void createHeader( );

    void setEventMode( uint8 mode, float32 fs );

    // opens output files for writing
    void open( );

    // closes and merges output files
    void close( );

    // write a single sample to the channel specified. once all channels in the data record are
    // full, the record is written to the file and reset.
    // if too many samples are written into the record, an exception is thrown
    // sample is expected to be in physical units
    void addSample( uint16 ch, double value );

    // same as add sample, but more efficient when more than 1 sample should be written into the channel
    // samples are expected to be in physical units
    void blitSamples( uint16 ch, double *values, size_t n );

    void addRawSample( uint16 ch, void *value );
    void blitRawSamples( uint16 ch, void *values, size_t n );

    void addEvent( uint32 pos, uint16 type );
    void addEvent( uint32 pos, uint16 type, uint16 chan, uint32 duration );
    void addEvent( uint32 pos, uint16 type, uint16 chan, float32 value );

    GDFHeader *getHeader( );
    GDFSignalHeader *getSignalHeader( );
    GDFTagHeader *getTagHeader( );

private:
    std::vector< uint32 > signal_types;         // type of each normal signal
    std::vector< uint32 > signal_samplerates;   // sample rate of each normal signal

    std::map<uint16,uint16> channel_mapping;

    std::string filename;
    std::fstream file_main;
    std::fstream file_events;

    bool force_rec_duration;
    uint32 rec_dur_num, rec_dur_den;

    GDFHeader *header;
    GDFSignalHeader *signal_header;
    GDFTagHeader *tag_header;

    // data write buffer
    GDFData::Record record;
    int64 num_records;

    // event info
    uint8 events_mode;
    float32 events_fs;
    size_t events_num;
};

#endif // GDFWRITER_H
