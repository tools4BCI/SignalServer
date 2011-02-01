#ifndef TIA_DATAPACKET_BUILDER_H
#define TIA_DATAPACKET_BUILDER_H

#include "tia/data_packet.h"
#include "tia/data_packet3.h"

#include "tia_datapacket.h"

#include "network/input_stream.h"

namespace tia
{

//-----------------------------------------------------------------------------
class TiADataPacketBuilder
{
public:
    tobiss::DataPacket buildFustyDataPacketFromStream (InputStream& input_stream);
    tobiss::DataPacket3 buildFustyDataPacket3FromStream (InputStream& input_stream);
};

}

#endif // TIA_DATAPACKET_BUILDER_H
