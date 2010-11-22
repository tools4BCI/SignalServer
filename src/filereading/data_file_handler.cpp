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

#include "filereading/data_file_handler.h"

namespace tobiss
{
using std::cout;
using std::endl;

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

using boost::lexical_cast;
using boost::bad_lexical_cast;

//-----------------------------------------------------------------------------

DataFileHandler::DataFileHandler(boost::asio::io_service& io, std::map<std::string, std::string> config)
  :io_service_(io), config_(config), td_(0)
{

  // check config (file present)

  // get Object from FileFactory
  // set configuration

}

//-----------------------------------------------------------------------------

void DataFileHandler::run()
{
  #ifdef DEBUG
    cout << "FileReader: run" << endl;
  #endif

}


//-----------------------------------------------------------------------------

void DataFileHandler::stop()
{
  #ifdef DEBUG
    cout << "FileReader: stop" << endl;
  #endif

}

//-----------------------------------------------------------------------------

DataPacket DataFileHandler::getDataPacket()
{
  #ifdef DEBUG
    cout << "FileReader: getSyncData" << endl;
  #endif

  DataPacket packet;

  return(packet);
}



//-----------------------------------------------------------------------------

void DataFileHandler::readSamples()
{
  #ifdef DEBUG
    cout << "FileReader: readSamples" << endl;
  #endif

}

//-----------------------------------------------------------------------------

} // Namespace tobiss
