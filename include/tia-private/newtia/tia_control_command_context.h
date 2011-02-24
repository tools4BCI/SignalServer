#ifndef TIA_CONTROL_COMMAND_CONTEXT_H
#define TIA_CONTROL_COMMAND_CONTEXT_H

#include "hardware_interface.h"
#include "server/data_server.h"

namespace tia
{

class TiAControlCommandContext
{
public:
    TiAControlCommandContext (HardwareInterface& hardware_interface)
        : hardware_interface_ (hardware_interface)
    {}

    bool hasDataConnection () const {return data_connections_.size () > 0;}

    std::vector<ConnectionID> getDataConnectionID () const {return data_connections_;}

    HardwareInterface& getHardwareInterface () {return hardware_interface_;}

    void addDataConnectionID (ConnectionID id) {data_connections_.push_back (id);}

private:
    HardwareInterface& hardware_interface_;
    std::vector<ConnectionID> data_connections_;
};



}

#endif // TIA_SERVER_CONTROL_CONNECTION_H
