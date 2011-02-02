#include "tia-private/server/commands/stop_data_transmission_server_command.h"

namespace tia
{

//-------------------------------------------------------------------------------------------------
StopDataTransmissionServerCommand::StopDataTransmissionServerCommand (ConnectionID const
                                                                      connection_id,
                                                                      DataServer& data_server,
                                                                      WriteSocket& socket)
    : ServerCommand (socket),
      connection_id_ (connection_id),
      socket_ (socket),
      data_server_ (data_server)
{

}

//-------------------------------------------------------------------------------------------------
void StopDataTransmissionServerCommand::execute ()
{
    if (data_server_.hasConnection (connection_id_))
    {
        if (data_server_.transmitting (connection_id_))
        {
            data_server_.stopTransmission (connection_id_);
            socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><okReply  /></message>");
        }
        else
            socket_.sendString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?><message version=\"0.2\"><errorReply  /></message>");
    }
}


}
