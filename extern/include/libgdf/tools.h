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

#ifndef TOOLS_H
#define TOOLS_H

#include "strexception.h"
#include "user_types.h"
#include <boost/numeric/conversion/cast.hpp>
#include <vector>

template<class T>
T GCD( T a, T b)
{
    if( a == 0 )
        return b;
    if( b == 0 )
        return a;
    while( 1 )
    {
        a = a % b;
        if( a == 0 )
            return b;

        b = b % a;
        if( b == 0 )
            return a;
    }
}

template<class T>
T GCD( const std::vector<T> &v, size_t p, size_t r )
{
    if( r == p+1 )
        return GCD( v[p], v[r] );

    if( r == p )
        return v[p];

    int q = (p+r)/2;
    return GCD( GCD( v, p, q ), GCD( v, q+1, r ) );
}

template<class T>
T GCD( const std::vector<T> &v )
{
    return GCD( v, size_t(0), v.size()-1 );
}

template<class T>
static T castGDF( void *dat, uint32 datatype )
{
    using boost::numeric_cast;
    switch( datatype )
    {
    default: throw StrException( "Unknown Data Type" ); break;

    case 1: return numeric_cast<T>( *reinterpret_cast<int8*>(dat) );
    case 2: return numeric_cast<T>( *reinterpret_cast<uint8*>(dat) );
    case 3: return numeric_cast<T>( *reinterpret_cast<int16*>(dat) );
    case 4: return numeric_cast<T>( *reinterpret_cast<uint16*>(dat) );
    case 5: return numeric_cast<T>( *reinterpret_cast<int32*>(dat) );
    case 6: return numeric_cast<T>( *reinterpret_cast<uint32*>(dat) );
    case 7: return numeric_cast<T>( *reinterpret_cast<int64*>(dat) );
    case 8: return numeric_cast<T>( *reinterpret_cast<uint64*>(dat) );

    case 16: return numeric_cast<T>( *reinterpret_cast<float32*>(dat) );
    case 17: return numeric_cast<T>( *reinterpret_cast<float64*>(dat) );
    case 18: throw StrException( "Cannot cast float128" ); break;

    case 279: throw StrException( "Cannot cast int24" ); break;
    case 535: throw StrException( "Cannot cast uint24" ); break;
    }
    return T(0);
}

void typecheck( );


#endif // TOOLS_H
