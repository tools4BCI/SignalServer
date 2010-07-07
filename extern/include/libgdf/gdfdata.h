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

#ifndef GDFDATA_H
#define GDFDATA_H

#include "user_types.h"
#include "strexception.h"
#include <vector>
#include <memory.h>
#include <iostream>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>

#define GDF_INT8_TYPE		1
#define GDF_UINT8_TYPE		2
#define GDF_INT16_TYPE		3
#define GDF_UINT16_TYPE		4
#define GDF_INT32_TYPE		5
#define GDF_UINT32_TYPE		6
#define GDF_INT64_TYPE		7
#define	GDF_UINT64_TYPE		8
#define GDF_FLOAT_TYPE		16
#define GDF_FLOAT32_TYPE	16
#define GDF_FLOAT64_TYPE	17
#define GDF_DOUBLE_TYPE		17
#define GDF_FLOAT128_TYPE	18
#define GDF_INT24_TYPE		279
#define GDF_UINT24_TYPE		535

class GDFData
{
public:
    GDFData();

    struct ChannelBase
    {
        ChannelBase( uint32 typ, size_t ns ) { datatype = typ; num_samples = ns; }
        virtual void assignBuffer( char *buf ) = 0;
        virtual size_t getSize( ) = 0;
        virtual size_t getFree( ) = 0;

        virtual bool isFull( ) = 0;
        virtual bool isEmpty( ) = 0;
        virtual void reset( ) = 0;

        char *getDataPtr( ) { return cdata; }

        template<class T>
        T getRawSample( size_t n )
        {
            using boost::numeric_cast;
            switch( datatype )
            {
            default: throw StrException( "Unknown Data Type" ); break;
            case 1 : return numeric_cast<T>( (reinterpret_cast<int8*>(cdata))[n] ); break;
            case 2 : return numeric_cast<T>( (reinterpret_cast<uint8*>(cdata))[n] ); break;
            case 3 : return numeric_cast<T>( (reinterpret_cast<int16*>(cdata))[n] ); break;
            case 4 : return numeric_cast<T>( (reinterpret_cast<uint16*>(cdata))[n] ); break;
            case 5 : return numeric_cast<T>( (reinterpret_cast<int32*>(cdata))[n] ); break;
            case 6 : return numeric_cast<T>( (reinterpret_cast<uint32*>(cdata))[n] ); break;
            case 7 : return numeric_cast<T>( (reinterpret_cast<int64*>(cdata))[n] ); break;
            case 8 : return numeric_cast<T>( (reinterpret_cast<uint64*>(cdata))[n] ); break;

            case 16 : return numeric_cast<T>( (reinterpret_cast<float32*>(cdata))[n] ); break;
            case 17 : return numeric_cast<T>( (reinterpret_cast<float64*>(cdata))[n] ); break;
            case 18 : throw StrException( "Don't know how to treat float128" ); break;

            case 279 : throw StrException( "Don't know how to treat int24" ); break;
            case 535 : throw StrException( "Don't know how to treat uint24" ); break;
            }
        }

        uint32 datatype;
        size_t num_samples;
        char *cdata;
    };

    template<class T>
    struct Channel : public ChannelBase
    {
        Channel( uint32 typ, size_t len ) : ChannelBase( typ, len )
        {
            writepos = NULL;
        }

        virtual void assignBuffer( char *buf )
        {
            data = (T*)buf;
            cdata = reinterpret_cast<char*>(data);
            writepos = data;
        }

        virtual size_t getSize( )
        {
            return num_samples * sizeof(T);
        }

        virtual size_t getFree( )
        {
            return num_samples+data-writepos;
        }

        virtual bool isFull( )
        {
            return writepos >= data+num_samples;
        }

        virtual bool isEmpty( )
        {
            return writepos == data;
        }

        virtual void reset( )
        {
            writepos = data;
        }

        // multiple samples
        void blit( T *dat, size_t n )
        {
            if( writepos+n > data+num_samples )
                throw StrException( "Channel Buffer Overflow!" );
            memcpy( writepos, dat, sizeof(T)*n );
            writepos += n;
        }

        // single sample
        void blit( T dat )
        {
            if( writepos >= data+num_samples )
                throw StrException( "Channel Buffer Overflow!" );
            *writepos = dat;
            writepos++;
        }

        // fill multiple samples
        void fill( size_t n )
        {
            if( writepos+n > data+num_samples )
                throw StrException( "Channel Buffer Overflow!" );
            memset( writepos, 0, sizeof(T)*n );
            writepos += n;
        }

        T *data;
        T *writepos;
    };

    struct Record
    {
        Record( )
        {
            buffer = NULL;
            bufend = NULL;
            bufsize = 0;
        }

        ~Record( )
        {
            if( buffer )
                delete[] buffer;
        }

        // copy constructor
        Record( const Record &other )
        {
            std::vector<ChannelBase*>::const_iterator it = other.channels.begin( );
            for( ; it!=other.channels.end(); it++ )
            {
                addChannel( (*it)->datatype, (*it)->num_samples );
            }
            createBuffer( );
            assignBuffer( );
            memcpy( buffer, other.buffer, bufsize );
        }

        void createBuffer( )
        {
            buffer = new char[bufsize];
            bufend = buffer + bufsize;
        }

        void destroyBuffer( )
        {
            if( buffer )
                delete[] buffer;
            buffer = NULL;
            bufend = NULL;
            bufsize = 0;
        }

        void assignBuffer( )
        {
            char *bufpos = buffer;
            for( size_t ch=0; ch<channels.size(); ch++ )
            {
                ChannelBase *chan = channels[ch];
                size_t len = chan->getSize( );
                chan->assignBuffer( bufpos );
                bufpos += len;
                if( bufpos > bufend )
                    throw StrException( "Record Buffer Overflow!" );
            }
        }

        void reset( )
        {
            std::vector<ChannelBase*>::iterator it = channels.begin( );
            for( ; it!=channels.end(); it++ )
                (*it)->reset( );
        }

        static bool checkDataType( uint32 datatype )
        {
            switch( datatype )
            {
            default: return false;
            case 1 :
            case 2 :
            case 3 :
            case 4 :
            case 5 :
            case 6 :
            case 7 :
            case 8 :

            case 16 :
            case 17 :
            case 18 :

            case 279 :
            case 535 : break;
            }
            return true;
        }

        void addChannel( uint32 datatype, uint32 samples_per_record )
        {
            ChannelBase *chan = NULL;
            switch( datatype )
            {
            default: throw StrException( "Unknown Data Type" ); break;
            case 1 : chan = new Channel<int8>( datatype, samples_per_record ); break;
            case 2 : chan = new Channel<uint8>( datatype, samples_per_record ); break;
            case 3 : chan = new Channel<int16>( datatype, samples_per_record ); break;
            case 4 : chan = new Channel<uint16>( datatype, samples_per_record ); break;
            case 5 : chan = new Channel<int32>( datatype, samples_per_record ); break;
            case 6 : chan = new Channel<uint32>( datatype, samples_per_record ); break;
            case 7 : chan = new Channel<int64>( datatype, samples_per_record ); break;
            case 8 : chan = new Channel<uint64>( datatype, samples_per_record ); break;

            case 16 : chan = new Channel<float32>( datatype, samples_per_record ); break;
            case 17 : chan = new Channel<float64>( datatype, samples_per_record ); break;
            case 18 : chan = new Channel<float128>( datatype, samples_per_record ); break;

            case 279 : chan = new Channel<int24>( datatype, samples_per_record ); break;
            case 535 : chan = new Channel<uint24>( datatype, samples_per_record ); break;
            }
            channels.push_back( chan );
            bufsize += chan->getSize( );
        }

        // write n samples into channel
        void blit( size_t ch, void *dat, size_t n )
        {
            ChannelBase *chan = channels[ch];
            try
            {
                switch( chan->datatype )
                {
                default: throw StrException( "Unknown Data Type" ); break;
                case 1: dynamic_cast<Channel<int8>*>(chan)->blit( reinterpret_cast<int8*>(dat), n ); break;
                case 2: dynamic_cast<Channel<uint8>*>(chan)->blit( reinterpret_cast<uint8*>(dat), n ); break;
                case 3: dynamic_cast<Channel<int16>*>(chan)->blit( reinterpret_cast<int16*>(dat), n ); break;
                case 4: dynamic_cast<Channel<uint16>*>(chan)->blit( reinterpret_cast<uint16*>(dat), n ); break;
                case 5: dynamic_cast<Channel<int32>*>(chan)->blit( reinterpret_cast<int32*>(dat), n ); break;
                case 6: dynamic_cast<Channel<uint32>*>(chan)->blit( reinterpret_cast<uint32*>(dat), n ); break;
                case 7: dynamic_cast<Channel<int64>*>(chan)->blit( reinterpret_cast<int64*>(dat), n ); break;
                case 8: dynamic_cast<Channel<uint64>*>(chan)->blit( reinterpret_cast<uint64*>(dat), n ); break;

                case 16: dynamic_cast<Channel<float32>*>(chan)->blit( reinterpret_cast<float32*>(dat), n ); break;
                case 17: dynamic_cast<Channel<float64>*>(chan)->blit( reinterpret_cast<float64*>(dat), n ); break;
                case 18: dynamic_cast<Channel<float128>*>(chan)->blit( reinterpret_cast<float128*>(dat), n ); break;

                case 279: dynamic_cast<Channel<int24>*>(chan)->blit( reinterpret_cast<int24*>(dat), n ); break;
                case 535: dynamic_cast<Channel<uint24>*>(chan)->blit( reinterpret_cast<uint24*>(dat), n ); break;
                }
            }
            catch( std::exception &e )
            {
				throw StrException( e.what() + std::string(" in Channel ") + boost::lexical_cast<std::string>(ch) );
            }
        }

        // write 1 sample into channel
        void blit( size_t ch, void *dat )
        {
            ChannelBase *chan = channels[ch];
            try
            {
                switch( chan->datatype )
                {
                default: throw StrException( "Unknown Data Type" ); break;
                case 1: dynamic_cast<Channel<int8>*>(chan)->blit( *reinterpret_cast<int8*>(dat) ); break;
                case 2: dynamic_cast<Channel<uint8>*>(chan)->blit( *reinterpret_cast<uint8*>(dat) ); break;
                case 3: dynamic_cast<Channel<int16>*>(chan)->blit( *reinterpret_cast<int16*>(dat) ); break;
                case 4: dynamic_cast<Channel<uint16>*>(chan)->blit( *reinterpret_cast<uint16*>(dat) ); break;
                case 5: dynamic_cast<Channel<int32>*>(chan)->blit( *reinterpret_cast<int32*>(dat) ); break;
                case 6: dynamic_cast<Channel<uint32>*>(chan)->blit( *reinterpret_cast<uint32*>(dat) ); break;
                case 7: dynamic_cast<Channel<int64>*>(chan)->blit( *reinterpret_cast<int64*>(dat) ); break;
                case 8: dynamic_cast<Channel<uint64>*>(chan)->blit( *reinterpret_cast<uint64*>(dat) ); break;

                case 16: dynamic_cast<Channel<float32>*>(chan)->blit( *reinterpret_cast<float32*>(dat) ); break;
                case 17: dynamic_cast<Channel<float64>*>(chan)->blit( *reinterpret_cast<float64*>(dat) ); break;
                case 18: dynamic_cast<Channel<float128>*>(chan)->blit( *reinterpret_cast<float128*>(dat) ); break;

                case 279: dynamic_cast<Channel<int24>*>(chan)->blit( *reinterpret_cast<int24*>(dat) ); break;
                case 535: dynamic_cast<Channel<uint24>*>(chan)->blit( *reinterpret_cast<uint24*>(dat) ); break;
                }
            }
            catch( std::exception &e )
            {
                throw StrException( e.what() + std::string(" in Channel ") + boost::lexical_cast<std::string>(ch) );
            }
        }

        // write n samples into channel, cast from double
        void blit( size_t ch, double *dat, size_t n )
        {
            for( size_t i=0; i<n; i++)
                blit( ch, dat[i] );
        }

        // write 1 sample into channel, cast from double
        void blit( size_t ch, double dat )
        {
            using boost::numeric_cast;
            ChannelBase *chan = channels[ch];
            try
            {
                switch( chan->datatype )
                {
                default: throw StrException( "Unknown Data Type" ); break;
                case 1: dynamic_cast<Channel<int8>*>(chan)->blit( numeric_cast<int8>(dat) ); break;
                case 2: dynamic_cast<Channel<uint8>*>(chan)->blit( numeric_cast<uint8>(dat) ); break;
                case 3: dynamic_cast<Channel<int16>*>(chan)->blit( numeric_cast<int16>(dat) ); break;
                case 4: dynamic_cast<Channel<uint16>*>(chan)->blit( numeric_cast<uint16>(dat) ); break;
                case 5: dynamic_cast<Channel<int32>*>(chan)->blit( numeric_cast<int32>(dat) ); break;
                case 6: dynamic_cast<Channel<uint32>*>(chan)->blit( numeric_cast<uint32>(dat) ); break;
                case 7: dynamic_cast<Channel<int64>*>(chan)->blit( numeric_cast<int64>(dat) ); break;
                case 8: dynamic_cast<Channel<uint64>*>(chan)->blit( numeric_cast<uint64>(dat) ); break;

                case 16: dynamic_cast<Channel<float32>*>(chan)->blit( numeric_cast<float32>(dat) ); break;
                case 17: dynamic_cast<Channel<float64>*>(chan)->blit( numeric_cast<float64>(dat) ); break;
                case 18: throw StrException( "Don't know how to treat float128" ); break;

                case 279: throw StrException( "Don't know how to treat int24" ); break;
                case 535: throw StrException( "Don't know how to treat uint24" ); break;
                }
            }
            catch( std::exception &e )
            {
                throw StrException( e.what() + std::string(" in Channel ") + boost::lexical_cast<std::string>(ch) );
            }
        }

        // set next n samples to the value in dat
        void fill( size_t ch, size_t n )
        {
            ChannelBase *chan = channels[ch];
            try
            {
                switch( chan->datatype )
                {
                default: throw StrException( "Unknown Data Type" ); break;
                case 1: dynamic_cast<Channel<int8>*>(chan)->fill( n ); break;
                case 2: dynamic_cast<Channel<uint8>*>(chan)->fill( n ); break;
                case 3: dynamic_cast<Channel<int16>*>(chan)->fill( n ); break;
                case 4: dynamic_cast<Channel<uint16>*>(chan)->fill( n ); break;
                case 5: dynamic_cast<Channel<int32>*>(chan)->fill( n ); break;
                case 6: dynamic_cast<Channel<uint32>*>(chan)->fill( n ); break;
                case 7: dynamic_cast<Channel<int64>*>(chan)->fill( n ); break;
                case 8: dynamic_cast<Channel<uint64>*>(chan)->fill( n ); break;

                case 16: dynamic_cast<Channel<float32>*>(chan)->fill( n ); break;
                case 17: dynamic_cast<Channel<float64>*>(chan)->fill( n ); break;
                case 18: dynamic_cast<Channel<float128>*>(chan)->fill( n ); break;

                case 279: dynamic_cast<Channel<int24>*>(chan)->fill( n ); break;
                case 535: dynamic_cast<Channel<uint24>*>(chan)->fill( n ); break;
                }
            }
            catch( std::exception &e )
            {
                throw StrException( e.what() + std::string(" in Channel ") + boost::lexical_cast<std::string>(ch) );
            }
        }

        void fill( )
        {
            for( size_t ch=0; ch<channels.size(); ch++ )
            {
                fill( ch, channels[ch]->getFree( ) );
            }
        }

        bool isFull( )
        {
            std::vector<ChannelBase*>::iterator it = channels.begin( );
            for( ; it!=channels.end(); it++ )
            {
                if( !(*it)->isFull( ) )
                    return false;
            }
            return true;
        }

        bool isEmpty( )
        {
            std::vector<ChannelBase*>::iterator it = channels.begin( );
            for( ; it!=channels.end(); it++ )
            {
                if( !(*it)->isEmpty( ) )
                    return false;
            }
            return true;
        }

        char *buffer;
        char *bufend;
        size_t bufsize;
        std::vector<ChannelBase*> channels;
    };

private:
};

#endif // GDFDATA_H
