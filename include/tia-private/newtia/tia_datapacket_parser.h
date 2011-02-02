#ifndef TIA_DATAPACKET_PARSER_H
#define TIA_DATAPACKET_PARSER_H

#include "tia/data_packet.h"
#include "tia/data_packet3.h"

#include "tia_datapacket.h"

#include "network/input_stream.h"

namespace tia
{

//-----------------------------------------------------------------------------
class TiADataPacketParser
{
public:
    tobiss::DataPacket parseFustyDataPacketFromStream (InputStream& input_stream);
    tobiss::DataPacket3 parseFustyDataPacket3FromStream (InputStream& input_stream);
};

}

#endif // TIA_DATAPACKET_PARSER_H
