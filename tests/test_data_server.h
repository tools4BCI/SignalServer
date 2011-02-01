#ifndef TEST_DATA_SERVER_H
#define TEST_DATA_SERVER_H

#include "tia-private/newtia/data_server.h"

#include <set>

//-----------------------------------------------------------------------------
class TestDataServer : public tia::DataServer
{
public:
    virtual ~TestDataServer () {}

    virtual tia::Port localPort (tia::ConnectionID /*connection*/) const {return 1000;}

    virtual tia::ConnectionID addConnection (bool /*udp*/)
    {
        tia::ConnectionID id = next_free_id_++;
        connections_.insert(id);
        return id;
    }

    virtual bool hasConnection (tia::ConnectionID connection) const {return connections_.count (connection);}

    virtual bool transmitting (tia::ConnectionID connection) const {return transmitting_.count (connection);}

    virtual bool removeConnection (tia::ConnectionID connection) {connections_.erase (connection); transmitting_.erase (connection); return true;}

    virtual void startTransmission (tia::ConnectionID connection) {transmitting_.insert (connection);}

    virtual void stopTransmission (tia::ConnectionID connection) {transmitting_.erase (connection);}

private:
    tia::ConnectionID next_free_id_;
    std::set<tia::ConnectionID> connections_;
    std::set<tia::ConnectionID> transmitting_;
};



#endif // TEST_DATA_SERVER_H
