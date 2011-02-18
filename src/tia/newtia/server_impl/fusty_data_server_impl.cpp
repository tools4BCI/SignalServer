#include "tia-private/newtia/server_impl/fusty_data_server_impl.h"
#include "tia-private/newtia/tia_exceptions.h"

#include <iostream>

namespace tia
{

//-----------------------------------------------------------------------------
FustyDataServerImpl::FustyDataServerImpl (tobiss::TCPDataServer& tcp_data_server,
                                          tobiss::UDPDataServer& udp_data_server)
    : tcp_data_server_ (tcp_data_server),
      udp_data_server_ (udp_data_server),
      next_free_connection_id_ (0)
{

}

//-----------------------------------------------------------------------------
FustyDataServerImpl::~FustyDataServerImpl ()
{

}

//-----------------------------------------------------------------------------
Port FustyDataServerImpl::localPort (ConnectionID connection) const
{
    if (tcp_connections_.count (connection))
        return id_tcp_endpoint_map_.at (connection).port ();
    else
        return udp_data_server_.destination ().port ();

    throw TiAException ("FustyDataServerImpl::localPort: No connection with that id.");
}

//-----------------------------------------------------------------------------
ConnectionID  FustyDataServerImpl::addConnection (bool udp)
{
    ConnectionID new_connection_id = next_free_connection_id_;
    next_free_connection_id_++;

    if (udp)
    {
        udp_data_server_.incClientCount ();
        udp_connections_.insert (new_connection_id);
    }
    else
    {
        // std::cout << "call addconneciton on tcp server" << std::endl;
        boost::asio::ip::tcp::endpoint tcp_endpoint = tcp_data_server_.addConnection ();
        // std::cout << "call addconneciton on tcp server done" << std::endl;
        tcp_connections_.insert (new_connection_id);
        id_tcp_endpoint_map_[new_connection_id] = tcp_endpoint;
    }

    return new_connection_id;
}

//-----------------------------------------------------------------------------
bool FustyDataServerImpl::hasConnection (ConnectionID connection) const
{
    if (tcp_connections_.count (connection))
        return true;
    if (udp_connections_.count (connection))
        return true;

    return false;
}

//-----------------------------------------------------------------------------
bool FustyDataServerImpl::transmitting (ConnectionID connection) const
{
    return transmitting_.count (connection);
}

//-----------------------------------------------------------------------------
bool FustyDataServerImpl::removeConnection (ConnectionID connection)
{
    if (tcp_connections_.count (connection))
    {
        tcp_connections_.erase (connection);
        tcp_data_server_.removeConnection (id_tcp_endpoint_map_[connection]);
        id_tcp_endpoint_map_.erase (connection);
        return true;
    }
    else if (udp_connections_.count (connection))
    {
        udp_connections_.erase (connection);
        udp_data_server_.decClientCount ();
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
void FustyDataServerImpl::startTransmission (ConnectionID connection)
{
    if (tcp_connections_.count (connection))
        tcp_data_server_.enableTransmission (id_tcp_endpoint_map_[connection], true);

    transmitting_.insert (connection);
}

//-----------------------------------------------------------------------------
void FustyDataServerImpl::stopTransmission (ConnectionID connection)
{
    if (tcp_connections_.count (connection))
        tcp_data_server_.enableTransmission (id_tcp_endpoint_map_[connection], false);

    transmitting_.erase (connection);
}



}
