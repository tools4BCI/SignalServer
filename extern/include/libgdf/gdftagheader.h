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

#ifndef GDFTAGHEADER_H
#define GDFTAGHEADER_H

#include "strexception.h"

#include "user_types.h"

#include <memory.h>
#include <vector>
#include <iostream>

class GDFTagHeader
{
public:
    class Tag
    {
    public:
        Tag( const void *buf )
        {
            tag = reinterpret_cast<const uint8*>(buf)[0];
            length[0] = reinterpret_cast<const uint8*>(buf)[1];
            length[1] = reinterpret_cast<const uint8*>(buf)[2];
            length[2] = reinterpret_cast<const uint8*>(buf)[3];

            value = new char[getLength()];
            memcpy( value, &reinterpret_cast<const uint8*>(buf)[4], getLength() );
        }

        Tag( uint8 t, uint32 len, const void *val )
        {
            tag = t;
            while( len > UINT24_MAX )
                len -= UINT24_MAX+1;
            length[0] = len % 256;
            length[1] = ((len - length[0])/256) % 256;
            length[2] = ((len - length[0] - length[1]*256)/256) % 256;
            value = new char[getLength()];
            memcpy( value, val, getLength() );
        }

        Tag( const Tag &other )
        {
            tag = other.tag;
            length[0] = other.length[0];
            length[1] = other.length[1];
            length[2] = other.length[2];

            value = new char[getLength()];
            memcpy( value, other.value, getLength() );
        }

        ~Tag( )
        {
            delete[] value;
        }

        uint8 getTag( ) { return tag; }

        uint32 getLength( )
        {
            return length[0] + 256*length[1] + 65536*length[2];
        }

        uint24 *getLengthRef( )
        {
            return (uint24*)length;
        }

        void *getVal( ) { return value; }

    private:
        uint8 tag;
        uint8 length[3];
        char *value;
    };

    GDFTagHeader()
    {
        buffer = NULL;
        length = 0;
    }

    ~GDFTagHeader( )
    {
        if( buffer )
            delete[] buffer;
    }

    void createBuffer( )
    {
        if( buffer )
            delete[] buffer;

        buffer = NULL;
        length = 0;

        if( tlv.size() > 0)
        {
            std::vector<Tag>::iterator it = tlv.begin( );
            for( ; it != tlv.end(); it++ )
                length += it->getLength( );

            // make sure length is a multiple of 256
            if( length % 256 != 0)
                length += 256 - length % 256;

            buffer = new char[length];
            memset( buffer, 0, length );

            char *p = buffer;

            it = tlv.begin( );
            for( ; it != tlv.end(); it++ )
            {
                *p = (char)it->getTag( );
                p++;
                *(uint24*)p = *it->getLengthRef( );
                p += 3;
                memcpy( p, it->getVal( ), it->getLength( ) );
                p += it->getLength( );
            }
        }
    }

    void fromBuffer( char *buf, size_t len )
    {
        buffer = buf;
        length = len;

        if( !buffer )
            throw StrException( "Buffer not initialized" );

        if( length == 0 )
            return;

        char *p = buffer;

        while( 1 )
        {
            Tag tag( p );
            tlv.push_back( tag );
            p += 4+tag.getLength( );

            if( p >= buffer + length - 3)
                break;

            if( *p == 0 )
                break;
        }

    }

    void addTag( Tag &tag )
    {
        tlv.push_back( tag );
    }

    void addTag( uint8 t, uint32 len, const void *val )
    {
        tlv.push_back( Tag( t, len, val ) );
    }

    size_t getSize( ) { return tlv.size( ); }

    Tag *getTag( size_t i ) { return &tlv[i]; }

    std::vector<Tag> &getTLV( ) { return tlv; }

    char *getBuffer( ) { return buffer; }
    size_t bufferLength( ) { return length; }

private:
    char *buffer;
    size_t length;
    std::vector<Tag> tlv;
};

#endif // GDFTAGHEADER_H
