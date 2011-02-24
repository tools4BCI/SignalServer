#ifndef BOOST_UDP_READ_SOCKET_H
#define BOOST_UDP_READ_SOCKET_H

#include "tia-private/newtia/network/read_socket.h"

#include <boost/asio/ip/udp.hpp>
#include <memory>

namespace tia
{

class BoostUDPReadSocket : public ReadSocket
{
public:
    BoostUDPReadSocket (boost::asio::io_service& io_service, boost::asio::ip::udp::endpoint const& endpoint, unsigned buffer_size);
    virtual void setReceiveBufferSize (unsigned size);


    virtual std::string readLine (unsigned max_length);
    virtual std::string readString (unsigned length);
    virtual char readCharacter ();
    virtual void waitForData ();
private:
    void readBytes (unsigned num_bytes);
    std::auto_ptr<boost::asio::ip::udp::socket> socket_;
    std::string buffered_string_;
    unsigned buffer_size_;
    std::vector<char> rec_buffer_;
};

}

#endif // BOOST_UDP_READ_SOCKET_H
