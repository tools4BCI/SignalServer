
/**
* @file event_listener.h
*
* @brief a very first udp listener for events provided as sample block
*
**/

#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <boost/cstdint.hpp>

#include "hw_thread.h"

using namespace std;


//-----------------------------------------------------------------------------
class EventListener : public HWThread
{
  public:
	/**
    * @brief Constructor
    */
    EventListener(boost::asio::io_service& io, XMLParser& parser);

    /**
    * @brief Destructor
    */
    virtual ~EventListener();

    virtual SampleBlock<double> getAsyncData();
    /**
    * @brief Method to start listening for events.
    */
    virtual void run();
    /**
    * @brief Method to stop listening for events.
    */
    virtual void stop();

//-----------------------------------------------
  private:
    virtual SampleBlock<double> getSyncData()
    { return(data_); }

    void listen4Events(const boost::system::error_code& error,
                        std::size_t bytes_transferred);

    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const& )
    {  }

    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const&)
    {  }

//-----------------------------------------------

  private:
    bool initialized_;   ///< to check, if condition variable has been set at least once (otherwise deadlock)

//     TCPEventListener    tcp_event_listener_;
    boost::asio::ip::udp::socket    event_socket_udp_;

    boost::mutex sync_mut_;  ///< mutex neede for synchronisation
    boost::condition_variable_any cond_;   ///< condition variable to wake up getSyncData()

    vector<char> buffer_;
    vector<double> events_; ///< temporary vector holding recent samples of the sine (1 element per channel)
};

#endif // EVENTLISTENER_H

//-----------------------------------------------------------------------------
