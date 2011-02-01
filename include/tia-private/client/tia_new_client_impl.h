#ifndef TIA_NEW_CLIENT_IMPL_H
#define TIA_NEW_CLIENT_IMPL_H

#include "tia_client_impl_base.h"
#include "tia/ssconfig.h"
#include "../newtia/socket.h"
#include "../newtia/tia_control_message_builder.h"
#include "../newtia/tia_control_message_parser.h"

#include <memory>
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

    boost::asio::io_service io_service_;
    std::auto_ptr<Socket> socket_;
    std::auto_ptr<ReadSocket> data_socket_;
    std::auto_ptr<TiAControlMessageBuilder> message_builder_;
    std::auto_ptr<TiAControlMessageParser> message_parser_;
};


}

#endif // TIA_NEW_CLIENT_IMPL_H
