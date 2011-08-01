#ifndef TID_CLIENT_H
#define TID_CLIENT_H

#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

//-----------------------------------------------------------------------------

class IDMessage;

namespace TiD
{

class InputStream;
class TiDMessageParser;
class TiDMessageBuilder;

class TiDClient
{
  public:
    TiDClient();
    virtual ~TiDClient();

    void connect(std::string ip_addr, unsigned int port);

    void startReceiving();
    void stopReceiving();

    void setBufferSize (size_t size);

    void sendMessage(std::string tid_xml_context);
    void getLastMessagesContexts( std::vector< IDMessage >& messages  );

  private:
    /**
     * @brief Handle completion of a receive operation.
     */
    void receive();

    /**
     * @brief Handle completion of a write operation.
     */
    void handleWrite(const boost::system::error_code& e,
        std::size_t bytes_transferred);

    void disconnect();

    enum ConnectionState
    {
      State_Connected,
      State_Running,
      State_Stopped,
      State_ConnectionClosed
    };

    ConnectionState                                   state_;
    std::vector <IDMessage>                         messages_;

    boost::asio::io_service                           io_service_;
//    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket                      socket_;
    boost::mutex                                      mutex_;

    InputStream*                                      input_stream_;
    TiDMessageParser*                                 msg_parser_;
    TiDMessageBuilder*                                msg_builder_;
    boost::thread*                                    receive_thread_;
    boost::thread*                                    io_service_thread_;
};

//-----------------------------------------------------------------------------

}  //TiD

#endif // TID_CLIENT_H
