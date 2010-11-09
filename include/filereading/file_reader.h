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
* @file file_reader.h
*
* @brief
*
**/

#ifndef FILEREADER_H
#define FILEREADER_H

#include <vector>
#include <map>

#include <boost/cstdint.hpp>

#include "definitions/defines.h"
#include "datapacket/data_packet.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class FileReader
*
* @brief
*
*/
class FileReader
{
  public:
    /**
    *
    */
    FileReader();

    /**
    * @brief Destructor
    */
    virtual ~FileReader()    {   }


    virtual DataPacket getDataPacket() = 0;

    /**
    * @brief Method to start data acquisition.
    */
    void run();
    /**
    * @brief Method to stop data acquisition.
    */
    void stop();

    void readSamples();

//-----------------------------------------------
  private:



//-----------------------------------------------

  private:


};

} // Namespace tobiss

#endif // FILEREADER_H

//-----------------------------------------------------------------------------
