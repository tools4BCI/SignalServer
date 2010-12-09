#ifndef STOP_DATA_TRANSMISSION_SERVER_COMMAND_H
#define STOP_DATA_TRANSMISSION_SERVER_COMMAND_H

#include "server_command.h"

namespace tia
{

class StopDataTransmissionServerCommand : public ServerCommand
{
public:
    void execute (Socket&)
    {
        // stop the data transmission
    }

};

}

#endif // STOP_DATA_TRANSMISSION_SERVER_COMMAND_H
