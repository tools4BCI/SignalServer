#ifndef TIA_CONTROL_COMMAND_CONTEXT_H
#define TIA_CONTROL_COMMAND_CONTEXT_H

#include "hardware_interface.h"
#include "data_server.h"

namespace tia
{

class TiAControlCommandContext
{
public:
    TiAControlCommandContext (HardwareInterface& hardware_interface)
        : hardware_interface_ (hardware_interface),
          has_connection_ (false)
    {}

    bool hasDataConnection () const {return has_connection_;}

    ConnectionID getDataConnectionID () const {return data_connection_;}

    HardwareInterface& getHardwareInterface () {return hardware_interface_;}

    void setDataConnectionID (ConnectionID id) {data_connection_ = id; has_connection_ = true;}

private:
    HardwareInterface& hardware_interface_;
    ConnectionID data_connection_;
    bool has_connection_;
};



}

#endif // TIA_SERVER_CONTROL_CONNECTION_H
