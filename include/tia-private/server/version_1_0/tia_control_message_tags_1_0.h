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

    std::string const GET_CONFIG = "GetConfig";
    std::string const CONFIG = "Config";

}

}

#endif // TIA_CONTROL_MESSAGE_TAGS_1_0_H
