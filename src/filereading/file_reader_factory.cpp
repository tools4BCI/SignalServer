/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file file_reader_factory.cpp
**/

#include "filereading/file_reader_factory.h"

namespace tobiss
{

//-----------------------------------------------------------------------------

FileReaderFactory::FileReaderFactory()
{

  // build map with all supported dataformats and their respective "basic" objects

}

//-----------------------------------------------------------------------------

FileReaderFactory::~FileReaderFactory()
{

  // delete all FilerReader Objects in the map

}

//-----------------------------------------------------------------------------

FileReader* FileReaderFactory::getFileReader(std::string filetype)
{

  // build map with all supported dataformats and their respective "basic" objects

  return(0);
}

//-----------------------------------------------------------------------------


} // Namespace tobiss
