#include "tia-private/client/tia_new_client_impl.h"

#include "tia-private/server/tia_meta_info_parse_and_build_functions.h"
#include "tia-private/server/version_1_0/tia_control_message_builder_1_0.h"
#include "tia-private/server/version_1_0/tia_control_message_parser_1_0.h"
#include "tia-private/server/messages/standard_control_messages.h"
#include "tia-private/server/version_1_0/tia_control_message_tags_1_0.h"
#include "tia-private/server/tia_meta_info_parse_and_build_functions.h"
#include "tia-private/server/string_utils.h"
#include "tia-private/server/boost_socket_impl.h"

using namespace tobiss;
using namespace std;

namespace tia
{

unsigned const MAX_LINE_LENGTH = 30;

//-----------------------------------------------------------------------------
TiANewClientImpl::TiANewClientImpl ()
    : MESSAGE_VERSION_ (TiAControlMessageTags10::VERSION),
      message_builder_ (new TiAControlMessageBuilder10),
      message_parser_ (new TiAControlMessageParser10)
{

}

//-----------------------------------------------------------------------------
void TiANewClientImpl::connect (const std::string& address, short unsigned port)
{
    boost::asio::ip::tcp::endpoint server_address (boost::asio::ip::address_v4::from_string (address), port);
    socket_.reset (new BoostTCPSocketImpl (io_service_, server_address));
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
    socket_.reset (0);
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
    sendMessage (GetDataConnectionTiAControlMessage (MESSAGE_VERSION_, use_udp_bc));
    TiAControlMessage dataconnection_message = waitForControlMessage (TiAControlMessageTags10::DATA_CONNECTION_PORT);
    unsigned port = toUnsigned (dataconnection_message.getParameters ());

}

//-----------------------------------------------------------------------------
bool TiANewClientImpl::receiving() const
{

}

//-----------------------------------------------------------------------------
void TiANewClientImpl::stopReceiving()
{

}

//-----------------------------------------------------------------------------
void TiANewClientImpl::getDataPacket (DataPacket& packet)
{

}

//-----------------------------------------------------------------------------
void TiANewClientImpl::setBufferSize (size_t size)
{

}

//-----------------------------------------------------------------------------
void TiANewClientImpl::sendMessage (TiAControlMessage const& message)
{
    if (socket_.get ())
        socket_->sendString (message_builder_->buildTiAMessage (GetMetaInfoTiAControlMessage (MESSAGE_VERSION_)));
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::waitForOKResponse ()
{
    waitForControlMessage (TiAControlMessageTags10::OK);
}

//-----------------------------------------------------------------------------
void TiANewClientImpl::waitForErrorResponse ()
{
    waitForControlMessage (TiAControlMessageTags10::ERROR);
}

//-----------------------------------------------------------------------------
TiAControlMessage TiANewClientImpl::waitForControlMessage (std::string const& command_name)
{
    if (!socket_.get ())
        throw TiAException ("TiANewClientImpl: Connection to server not initializsed.");

    TiAControlMessage message = message_parser_->parseMessage (*socket_);

    if (message.getVersion () != TiAControlMessageTags10::ID_AND_VERSION)
        throw std::runtime_error (string ("wrong server response: awaiting \"") + TiAControlMessageTags10::VERSION + "\" but was \"" + message.getVersion() + "\"");

    if (message.getCommand () != command_name)
        throw std::runtime_error (string ("wrong server response: awaiting \"") + command_name + "\" but was \"" + message.getCommand () + "\"");

    return message;
}




}
