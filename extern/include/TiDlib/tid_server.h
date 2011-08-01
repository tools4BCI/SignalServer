/**
* @file tid_server.h
*
* @brief \TODO.
*
**/

#ifndef TID_SERVER_H
#define TID_SERVER_H

// Boost
#include <boost/thread/mutex.hpp>
#include <boost/cstdint.hpp>

#include "tcp_server.h"
#include "tid_connection.h"

//-----------------------------------------------------------------------------

/**
* @class TiDServer
*
* @brief TODO
*/

class IDMessage;

namespace TiD
{

class TiDServer : public TCPServer
{
  public:

    typedef std::map<TiDConnection::ConnectionID, TiDConnection::pointer>     TiDConnHandlers;

    /**
     * @brief Constructor
     * @param
     * @throw
     */
    TiDServer(boost::asio::io_service& io_service);
    /**
   * @brief Destructor
   */
    virtual ~TiDServer();

    bool newMessagesAvailable();
    void getLastMessages(std::vector<IDMessage>& messages);

  protected:
    /**
     * @brief Handles a new client connection
     * @param new_connection the connection to the client
     */
    virtual void handleAccept(const TCPConnection::pointer& new_connection,
        const boost::system::error_code& error);

    void clientHasDisconnected(const TiDConnection::ConnectionID& id);

    void dispatchMsg(IDMessage msg, const TiDConnection::ConnectionID& src_id);

  private:

    bool running_;

    TiDConnHandlers         connections_;  ///< list holding handlers for each connected client
    boost::mutex            dispatch_mutex_;
    boost::mutex            erase_mutex_;

    std::vector< IDMessage >  messages_;

};

} // Namespace TiD

#endif //TID_SERVER_H

// End Of File
