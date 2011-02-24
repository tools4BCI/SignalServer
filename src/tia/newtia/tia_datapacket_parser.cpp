#include "tia-private/newtia/tia_datapacket_parser.h"

#include <string>
#include <iostream>

using std::string;

namespace tia
{

//-----------------------------------------------------------------------------
tobiss::DataPacket TiADataPacketParser::parseFustyDataPacketFromStream (InputStream& input_stream, bool& run)
{
    tobiss::DataPacket packet;
    char buffer[10000];
    unsigned available = 0;
    for (; (available < 32) && run; ++available)
        buffer[available] = input_stream.readCharacter ();

    unsigned bytes_needed = packet.getRequiredRawMemorySize (reinterpret_cast<void*>(buffer), available);
    while ((bytes_needed != available) && run)
    {
        buffer[available] = input_stream.readCharacter ();
        available++;
        bytes_needed = packet.getRequiredRawMemorySize (reinterpret_cast<void*>(buffer), available);
    }
    if (!run)
        return packet;
    return tobiss::DataPacket (reinterpret_cast<void*>(buffer));
}

//-----------------------------------------------------------------------------
tobiss::DataPacket3 TiADataPacketParser::parseFustyDataPacket3FromStream (InputStream& /*input_stream*/)
{
    return tobiss::DataPacket3 ();
}

}

