/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

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
    FustyDataServerImpl (TCPDataServer& tcp_data_server, UDPDataServer& udp_data_server);

    virtual ~FustyDataServerImpl ();

    virtual Port localPort (ConnectionID connection) const;
    virtual std::string getTargetIP () const;

    virtual ConnectionID addConnection (bool udp);

    virtual bool hasConnection (ConnectionID connection) const;

    virtual bool transmitting (ConnectionID connection) const;

    virtual bool removeConnection (ConnectionID connection);

    virtual void startTransmission (ConnectionID connection);

    virtual void stopTransmission (ConnectionID connection);

private:
    TCPDataServer& tcp_data_server_;
    UDPDataServer& udp_data_server_;

    std::map<ConnectionID, boost::asio::ip::tcp::endpoint> id_tcp_endpoint_map_;
    std::set<ConnectionID> tcp_connections_;
    std::set<ConnectionID> udp_connections_;
    std::set<ConnectionID> transmitting_;
    ConnectionID next_free_connection_id_;
};

}

#endif // FUSTY_DATA_SERVER_IMPL_H
