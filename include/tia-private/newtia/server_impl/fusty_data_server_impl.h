#ifndef FUSTY_DATA_SERVER_IMPL_H
#define FUSTY_DATA_SERVER_IMPL_H


#include "../../network/tcp_data_server.h"
#include "../../network/udp_data_server.h"

#include "../server/data_server.h"

#include <map>
#include <set>

namespace tia
{

//-----------------------------------------------------------------------------
class FustyDataServerImpl : public DataServer
{
public:
    FustyDataServerImpl (tobiss::TCPDataServer& tcp_data_server, tobiss::UDPDataServer& udp_data_server);

    virtual ~FustyDataServerImpl ();

    virtual Port localPort (ConnectionID connection) const;

    virtual ConnectionID addConnection (bool udp);

    virtual bool hasConnection (ConnectionID connection) const;

    virtual bool transmitting (ConnectionID connection) const;

    virtual bool removeConnection (ConnectionID connection);

    virtual void startTransmission (ConnectionID connection);

    virtual void stopTransmission (ConnectionID connection);

private:
    tobiss::TCPDataServer& tcp_data_server_;
    tobiss::UDPDataServer& udp_data_server_;

    std::map<ConnectionID, boost::asio::ip::tcp::endpoint> id_tcp_endpoint_map_;
    std::set<ConnectionID> tcp_connections_;
    std::set<ConnectionID> udp_connections_;
    std::set<ConnectionID> transmitting_;
    ConnectionID next_free_connection_id_;
};

}

#endif // FUSTY_DATA_SERVER_IMPL_H
