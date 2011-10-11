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

#ifndef TIA_NEW_CLIENT_IMPL_H
#define TIA_NEW_CLIENT_IMPL_H

#include "tia_client_impl_base.h"
#include "tia/ssconfig.h"
#include "../newtia/network/socket.h"
#include "../newtia/messages/tia_control_message_builder.h"
#include "../newtia/messages/tia_control_message_parser.h"

#include <boost/asio.hpp>

namespace tia
{

//-----------------------------------------------------------------------------
///
/// TiaNewClientImpl
///
/// Client implementation of TiA 1.0
///
class TiANewClientImpl : public tobiss::TiAClientImplBase
{
public:
    TiANewClientImpl ();

    virtual ~TiANewClientImpl()
    {}

    virtual void connect (const std::string& address, short unsigned port);

    virtual bool connected () const;

    virtual void disconnect ();

    virtual void requestConfig ();

    virtual tobiss::SSConfig config () const;

    virtual void startReceiving (bool use_udp_bc);

    virtual bool receiving() const;

    virtual void stopReceiving();

    virtual void getDataPacket (tobiss::DataPacket& packet);

    virtual void setBufferSize (size_t size);

private:
    void sendMessage (TiAControlMessage const& message);
    void waitForOKResponse ();
    void waitForErrorResponse ();


    //-------------------------------------------------------------------------
    TiAControlMessage waitForControlMessage (std::string const& command_name);


    std::string const MESSAGE_VERSION_;
    std::string server_ip_address_;
    bool receiving_;

    tobiss::SSConfig config_;
    unsigned buffer_size_;

    boost::asio::io_service io_service_;
    std::auto_ptr<Socket> socket_;
    std::auto_ptr<ReadSocket> data_socket_;
    std::auto_ptr<TiAControlMessageBuilder> message_builder_;
    std::auto_ptr<TiAControlMessageParser> message_parser_;
//    std::auto_ptr<TiAControlMessageParser> message_parser_;
};


}

#endif // TIA_NEW_CLIENT_IMPL_H
