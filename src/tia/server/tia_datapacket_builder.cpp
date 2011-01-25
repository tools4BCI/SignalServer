#include "tia-private/server/tia_datapacket_builder.h"

#include <string>

using std::string;

namespace tia
{

//-----------------------------------------------------------------------------
tobiss::DataPacket TiADataPacketBuilder::buildFustyDataPacketFromStream (InputStream& input_stream)
{
    tobiss::DataPacket packet;
    string data (1, input_stream.readCharacter ());
    while (packet.getRequiredRawMemorySize (reinterpret_cast<void*>(const_cast<char*>(data.c_str ())), data.size ()) == 0)
        data += input_stream.readCharacter ();
    return tobiss::DataPacket (reinterpret_cast<void*>(const_cast<char*>(data.c_str ())));
}

//-----------------------------------------------------------------------------
tobiss::DataPacket3 TiADataPacketBuilder::buildFustyDataPacket3FromStream (InputStream& /*input_stream*/)
{
    return tobiss::DataPacket3 ();
}

}

