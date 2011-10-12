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

#include "tia-private/client/tia_new_client_impl.h"
#include "tia/defines.h"

#include "tia-private/newtia/tia_meta_info_parse_and_build_functions.h"
#include "tia-private/newtia/messages_impl/tia_control_message_builder_1_0.h"
#include "tia-private/newtia/messages_impl/tia_control_message_parser_1_0.h"
#include "tia-private/newtia/messages/standard_control_messages.h"
#include "tia-private/newtia/messages/tia_control_message_tags_1_0.h"
#include "tia-private/newtia/tia_meta_info_parse_and_build_functions.h"
#include "tia-private/newtia/string_utils.h"
#include "tia-private/newtia/network_impl/boost_tcp_socket_impl.h"
#include "tia-private/newtia/network_impl/boost_udp_read_socket.h"



using namespace std;

namespace tia
{

unsigned const MAX_LINE_LENGTH = 50;

//-----------------------------------------------------------------------------
TiANewClientImpl::TiANewClientImpl ()
    : MESSAGE_VERSION_ (TiAControlMessageTags10::VERSION),
      receiving_ (false),
      buffer_size_ (BUFFER_SIZE),
      message_builder_ (new TiAControlMessageBuilder10),
      message_parser_ (new TiAControlMessageParser10),
      data_packet_parser(0)
{

}

//-----------------------------------------------------------------------------
void TiANewClientImpl::connect (const std::string& address, short unsigned port)
{
    server_ip_address_ = address;
    boost::asio::ip::tcp::endpoint server_address (boost::asio::ip::address_v4::from_string (address), port);
    socket_.reset (new BoostTCPSocketImpl (io_service_, server_address, buffer_size_));
    sendMessage (CheckProtocolVersionTiAControlMessage (MESSAGE_VERSION_));
    waitForOKResponse ();
}

//-----------------------------------------------------------------------------
bool TiANewClientImpl::connected () const
{
    return socket_.get ();
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::disconnect ()
{
    if (receiving_)
        stopReceiving ();
    if (data_socket_.get ())
        data_socket_.reset (0);
    socket_.reset (0);

    if(data_packet_parser)
    {
      delete data_packet_parser;
      data_packet_parser = 0;
    }
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::requestConfig ()
{
    sendMessage (GetMetaInfoTiAControlMessage (MESSAGE_VERSION_));
    TiAControlMessage metainfo_message = waitForControlMessage (TiAControlMessageTags10::METAINFO);
    config_ = parseTiAMetaInfoFromXMLString (metainfo_message.getContent ());
}

//-----------------------------------------------------------------------------
SSConfig TiANewClientImpl::config () const
{
    return config_;
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::startReceiving (bool use_udp_bc)
{
    if (receiving_)
        return;

    if (!data_socket_.get ())
    {
        sendMessage (GetDataConnectionTiAControlMessage (MESSAGE_VERSION_, use_udp_bc));
        TiAControlMessage dataconnection_message = waitForControlMessage (TiAControlMessageTags10::DATA_CONNECTION_PORT);
        unsigned port = toUnsigned (dataconnection_message.getParameters ());
        if (use_udp_bc)
        {
            boost::asio::ip::udp::endpoint server_data_address (boost::asio::ip::address_v4::from_string (server_ip_address_), port);
            data_socket_.reset (new BoostUDPReadSocket (io_service_, server_data_address, buffer_size_));
        }
        else
        {
            boost::asio::ip::tcp::endpoint server_data_address (boost::asio::ip::address_v4::from_string (server_ip_address_), port);
            data_socket_.reset (new BoostTCPSocketImpl (io_service_, server_data_address, buffer_size_));
        }
    }

    data_packet_parser =  new TiADataPacketParser(*data_socket_);

    sendMessage (TiAControlMessage (MESSAGE_VERSION_, TiAControlMessageTags10::START_DATA_TRANSMISSION, "", ""));
    waitForOKResponse ();

    receiving_ = true;
}

//-----------------------------------------------------------------------------
bool TiANewClientImpl::receiving() const
{
    return receiving_;
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::stopReceiving()
{
    if (receiving_)
    {
        sendMessage (TiAControlMessage (MESSAGE_VERSION_, TiAControlMessageTags10::STOP_DATA_TRANSMISSION, "", ""));
        waitForOKResponse();
        receiving_ = false;
    }
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::getDataPacket (DataPacketImpl& packet)
{
    if (!receiving_)
        return;

    if (!data_socket_.get ())
        return;

//    TiADataPacketParser packet_parser(*data_socket_);
//    packet = packet_parser.parseFustyDataPacketFromStream (*data_socket_, receiving_);

    data_packet_parser->parseDataPacket(packet);

    if (!receiving_)
        data_socket_.reset (0);
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::setBufferSize (size_t size)
{
    buffer_size_ = size;
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::sendMessage (TiAControlMessage const& message)
{
    if (socket_.get ())
        socket_->sendString (message_builder_->buildTiAMessage (message));
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::waitForOKResponse ()
{
    waitForControlMessage (TiAControlMessageTags10::OK);
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::waitForErrorResponse ()
{
    waitForControlMessage (TiAControlMessageTags10::ERROR_STR);
}

//-----------------------------------------------------------------------------
TiAControlMessage TiANewClientImpl::waitForControlMessage (std::string const& command_name)
{
    if (!socket_.get ())
        throw TiAException ("TiANewClientImpl: Connection to server not initializsed.");

    TiAControlMessage message = message_parser_->parseMessage (*socket_);

    if (message.getVersion () != TiAControlMessageTags10::VERSION)
        throw std::runtime_error (string ("wrong server response: awaiting \"") + TiAControlMessageTags10::VERSION + "\" but was \"" + message.getVersion() + "\"");

    if (message.getCommand () != command_name)
        throw std::runtime_error (string ("wrong server response: awaiting \"") + command_name + "\" but was \"" + message.getCommand () + "\"");

    return message;
}




}
