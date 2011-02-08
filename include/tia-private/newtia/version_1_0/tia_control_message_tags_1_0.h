#ifndef TIA_CONTROL_MESSAGE_TAGS_1_0_H
#define TIA_CONTROL_MESSAGE_TAGS_1_0_H

#include <string>

namespace tia
{

namespace TiAControlMessageTags10
{
    char const NEW_LINE_CHAR = 0x0A;
    std::string const NEW_LINE (1, NEW_LINE_CHAR);
    char const COMMAND_DELIMITER = ':';


    std::string const VERSION = "1.0";
    std::string const ID = "TiA";
    std::string const ID_AND_VERSION = ID + std::string (" ") + VERSION;

    std::string const CONTENT_LENGTH = "Content-Length";

    std::string const OK = "OK";
    std::string const ERROR = "Error";
    std::string const METAINFO = "MetaInfo";
    std::string const CHECK_PROTOCOL_VERSION = "CheckProtocolVersion";
    std::string const GET_METAINFO = "GetMetaInfo";
    std::string const DATA_CONNECTION_PORT = "DataConnectionPort";
    std::string const SERVER_STATE_CONNECTION_PORT = "ServerStateConnectionPort";
    std::string const GET_SERVER_STATE_CONNECTION = "GetServerStateConnection";
    std::string const GET_DATA_CONNECTION = "GetDataConnection";
    std::string const START_DATA_TRANSMISSION = "StartDataTransmission";
    std::string const STOP_DATA_TRANSMISSION = "StopDataTransmission";
    std::string const TCP = "TCP";
    std::string const UDP = "UDP";

}

}

#endif // TIA_CONTROL_MESSAGE_TAGS_1_0_H
