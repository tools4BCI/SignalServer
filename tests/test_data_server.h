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

#ifndef TEST_DATA_SERVER_H
#define TEST_DATA_SERVER_H

#include "tia-private/newtia/server/data_server.h"

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
