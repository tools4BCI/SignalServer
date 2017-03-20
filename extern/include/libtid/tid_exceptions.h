/*
    This file is part of TOBI Interface D (TiD).

    TOBI Interface D (TiD) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface D (TiD) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface D (TiD).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2012 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

#ifndef TID_EXCEPTIONS_H
#define TID_EXCEPTIONS_H

#include <stdexcept>

namespace TiD
{

class TiDException : public std::runtime_error
{
public:
    TiDException (std::string const& what) : std::runtime_error (what) {}
    virtual ~TiDException () throw () {}
};

class TiDLostConnection : public TiDException
{
public:
    TiDLostConnection (std::string const& what) : TiDException (what) {}
    virtual ~TiDLostConnection () throw () {}
};

}

#endif // TID_EXCEPTIONS_H
