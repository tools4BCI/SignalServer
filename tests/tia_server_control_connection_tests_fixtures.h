/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

#ifndef TIA_SERVER_CONTROL_CONNECTION_TESTS_FIXTURES_H
#define TIA_SERVER_CONTROL_CONNECTION_TESTS_FIXTURES_H

#include "tia-private/server/socket.h"
#include "tia-private/server/data_server.h"

#include "UnitTest++/UnitTest++.h"

#include "test_socket.h"

#include <map>



//-----------------------------------------------------------------------------
/// TestSocket
///
/// test implementation of the Socket class
class TestDataServer : public tia::DataServer
{
public:
    TestDataServer () : next_free_id_ (0), next_free_port_ (1234) {}

    unsigned connectionCount () const {return connection_transmission_map_.size ();}

    tia::ConnectionID mostRecentConnectionID () const {return most_recent_connection_id_;}

    bool transmissionEnabled (tia::ConnectionID connection) const {return connection_transmission_map_.at (connection);}

    virtual tia::Port localPort (tia::ConnectionID connection) const {return connection_port_map_.at (connection);}

    virtual tia::ConnectionID addConnection ()
    {
        most_recent_connection_id_ = next_free_id_;
        connection_transmission_map_.insert (std::pair<tia::ConnectionID, bool> (most_recent_connection_id_, false));
        connection_port_map_.insert (std::pair<tia::ConnectionID, tia::Port> (most_recent_connection_id_, next_free_port_));
        next_free_id_++;
        next_free_port_++;
        return most_recent_connection_id_;
    }

    virtual bool hasConnection (tia::ConnectionID connection) const
    {
        return connection_transmission_map_.count (connection) > 0;
    }

    virtual bool transmitting (tia::ConnectionID connection) const
    {
        bool transmit = false;
        if (connection_transmission_map_.count (connection))
            transmit = connection_transmission_map_.find (connection)->second;
        return transmit;
    }

    virtual bool removeConnection (tia::ConnectionID connection)
    {
        connection_transmission_map_.erase (connection);
        connection_port_map_.erase (connection);
        return true;
    }

    virtual void startTransmission (tia::ConnectionID connection) {if (connection_transmission_map_.count (connection)) connection_transmission_map_[connection] = true;}

    virtual void stopTransmission (tia::ConnectionID connection) {if (connection_transmission_map_.count (connection)) connection_transmission_map_[connection] = false;}

private:
    tia::ConnectionID next_free_id_;
    tia::Port next_free_port_;
    std::map<tia::ConnectionID, bool> connection_transmission_map_;
    std::map<tia::ConnectionID, tia::Port> connection_port_map_;
    tia::ConnectionID most_recent_connection_id_;
};

//-----------------------------------------------------------------------------
///
struct TiAServerControlConnectionFixture
{
    TestDataServer test_data_server;
    TestSocket test_control_socket;
};


#endif // TIA_SERVER_CONTROL_CONNECTION_TESTS_FIXTURES_H
