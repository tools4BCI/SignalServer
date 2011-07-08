/**
* @file tid_connection.h
* @brief TODO
**/

#ifndef TID_CONNECTION_H
#define TID_CONNECTION_H

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "tcp_server.h"

class IDMessage;

namespace TiD
{
  class TiDServer;
  class TiDConnection;

  class InputStream;
  class TiDMessageParser;
  class TiDMessageBuilder;

//-----------------------------------------------------------------------------

/**
* @class TiDConnection
*
* @brief TODO
*/

typedef boost::function< void(const std::pair<int, std::string>&) > deleteConnectionCallback;
typedef boost::function< void( IDMessage, const std::pair<int, std::string>& ) > dispatchTiDMessageCallback;

class TiDConnection : public boost::enable_shared_from_this<TiDConnection>
{
public:

  /**
   * @brief Control Connection Handle
   */
  typedef boost::shared_ptr<TiDConnection> pointer;
  typedef std::pair<int, std::string> ConnectionID;

  /**
   * @brief Creates a new ControlConnection
   * @return Handle pointing to the new control connection
   * @throws
   */
  static pointer create(const TCPConnection::pointer& tcp_conn_handle,
                        deleteConnectionCallback del_con_cb, dispatchTiDMessageCallback disp_msg_cb)
  {
    return pointer(new TiDConnection(tcp_conn_handle, del_con_cb, disp_msg_cb));
  }

  virtual ~TiDConnection();   /// Destructor



  void run();
  void stop();

  ConnectionID getID()
    { return(connection_id_); }

  /**
   * @brief Sends a TiD message to the client
   */
  void sendMsg(IDMessage& msg);

private:
  /// @brief Constructs a connection with the given io_service.
  TiDConnection(const TCPConnection::pointer& tcp_conn_handle,
                deleteConnectionCallback del_con_cb, dispatchTiDMessageCallback disp_msg_cb);

private:
  void receive();

  /**
   * @brief Handle completion of a write operation.
   */
  void handleWrite(const boost::system::error_code& e,
      std::size_t bytes_transferred);

  /**
   * @brief Closes the connection
   */
  void close();

private:
  static const int TID_MESSAGE_BUFFER_SIZE__IN_BYTE  =  16384; // ... 16kB
  ConnectionID                             connection_id_;
  TCPConnection::pointer                   tcp_connection_; ///<

  enum ConnectionState
  {
    State_Connected,
    State_Running,
    State_Stopped,
    State_ConnectionClosed
  };

  ConnectionState                          state_;                    ///<

  deleteConnectionCallback                 del_callback_ref_;
  dispatchTiDMessageCallback               disp_tid_msg_callback_ref_;

  std::vector<boost::uint8_t>              message_buffer_;

  InputStream*                             input_stream_;
  TiDMessageParser*                        msg_parser_;
  TiDMessageBuilder*                       msg_builder_;

  boost::thread*                           receive_thread_;

};

//-----------------------------------------------------------------------------

} // Namespace TiD

#endif //TID_CONNECTION_H

// End Of File
