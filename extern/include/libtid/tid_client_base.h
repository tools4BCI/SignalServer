/*
    This file is part of TOBI Interface D (TiD).

    TOBI Interface D (TiD) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface D (TiD) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface D (TiD).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2012 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

#ifndef TID_CLIENT_BASE_H
#define TID_CLIENT_BASE_H

#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#include "tobiid/IDMessage.hpp"

//-----------------------------------------------------------------------------

class IDMessage;

namespace TiD
{

class InputStream;
class TiDMessageParser;
class TiDMessageBuilder;

class TiDClientBase
    //: public boost::enable_shared_from_this<TiDClient>
{
  friend class TimedTiDClient;
  friend class LPTTiDClient;

  public:
    TiDClientBase();
    virtual ~TiDClientBase();

    void connect(std::string ip_addr, unsigned int port);
    void disconnect();

    virtual void startReceiving( bool throw_on_error = 0 ) = 0;
    virtual void stopReceiving() = 0;
    bool receiving();
    bool connected();

    void setBufferSize (size_t size);
    void reserveNrOfMsgs (size_t expected_nr_of_msgs);

    void sendMessage(std::string& tid_xml_context);
    void sendMessage(IDMessage& msg);

    void AsyncSendMessage(std::string& tid_xml_context);
    void AsyncSendMessage(IDMessage& msg);

    bool newMessagesAvailable();
    void getLastMessagesContexts( std::vector< IDMessage >& messages  );
    void clearMessages();

    IDMessage wait4NewTiDMessage();

  protected:
    /**
     * @brief Handle completion of a receive operation.
     */
    static int receive(void* instance);

    /**
     * @brief Handle completion of a write operation.
     */
    void handleWrite(const boost::system::error_code& e,
        std::size_t bytes_transferred);

    //void completionHandler(std::string& msg);


    enum ConnectionState
    {
      State_Connected,
      State_Running,
      State_Stopped,
      State_ConnectionClosed,
      State_Error
    };

    std::string                                       remote_ip_;
    unsigned int                                      remote_port_;

    ConnectionState                                   state_;
    std::string                                       xml_string_;
    std::vector <IDMessage>                           messages_from_net_;
    //IDMessage                                         msg_;

    boost::asio::ip::tcp::socket*                      socket_;
    boost::asio::io_service*                           io_service_;
    boost::mutex                                      mutex_net_msgs_;
    boost::mutex                                      state_mutex_;

    InputStream*                                      input_stream_;
    TiDMessageParser*                                 msg_parser_;
    TiDMessageBuilder*                                msg_builder_;

    bool                                              throw_on_error_;


};

//-----------------------------------------------------------------------------

}  //TiD

#endif // TID_CLIENT_BASE_H
