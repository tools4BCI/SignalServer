/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

#include <string>
#include <iostream>

#include "tia-private/newtia/tia_datapacket_parser.h"
#include "tia/defines.h"


using std::string;

namespace tia
{

static const int MINIMAL_DATA_PACKET_SIZE = 32;  /// todo: implement function in data packet

//-----------------------------------------------------------------------------

TiADataPacketParser::TiADataPacketParser(InputStream& input_stream)
  :input_stream_(input_stream)
{
  buffer_ = new char[BUFFER_SIZE];
}

//-----------------------------------------------------------------------------

TiADataPacketParser::~TiADataPacketParser()
{
  if(buffer_)
    delete[] buffer_;
}

//-----------------------------------------------------------------------------

void TiADataPacketParser::parseDataPacket (DataPacket& packet)
{
  size_t received = input_stream_.readBytes(buffer_,MINIMAL_DATA_PACKET_SIZE);

  size_t needed = packet.getRequiredRawMemorySize(buffer_,received);

  while(received < needed)
    received += input_stream_.readBytes(buffer_+received, needed - received);

  packet.reset(buffer_);
}

//-----------------------------------------------------------------------------

//DataPacketImpl TiADataPacketParser::parseFustyDataPacketFromStream (InputStream& input_stream, bool& run)
//{
//    DataPacketImpl packet;
//    char buffer[10000];
//    unsigned available = 0;
//    for (; (available < 32) && run; ++available)
//        buffer[available] = input_stream.readCharacter ();

//    unsigned bytes_needed = packet.getRequiredRawMemorySize (reinterpret_cast<void*>(buffer), available);
//    while ((bytes_needed != available) && run)
//    {
//        buffer[available] = input_stream.readCharacter ();
//        available++;
//        bytes_needed = packet.getRequiredRawMemorySize (reinterpret_cast<void*>(buffer), available);
//    }
//    if (!run)
//        return packet;
//    return DataPacketImpl (reinterpret_cast<void*>(buffer));
//}

//-----------------------------------------------------------------------------
//DataPacket3Impl TiADataPacketParser::parseFustyDataPacket3FromStream (InputStream& /*input_stream*/)
//{
//    return DataPacket3Impl ();
//}

}

