#ifndef TEST_SOCKET_H
#define TEST_SOCKET_H

#include "tia-private/newtia/network/socket.h"
#include "tia-private/newtia/network/tcp_server_socket.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <list>

//-----------------------------------------------------------------------------
/// TestSocket
///
/// test implementation of the Socket class
class TestSocket : public tia::Socket
{
public:
    TestSocket () : failure_if_sending_ (false) {}

    virtual void sendString (std::string const& string) throw (tia::TiALostConnection);

    virtual void setReceiveBufferSize (unsigned) {}
    virtual std::string readString (unsigned number_bytes = 4000000000u);
    virtual std::string readLine (unsigned max_length);
    virtual char readCharacter ();
    virtual void waitForData ();

    virtual std::string getRemoteEndPointAsString()
    {
      return("RemoteIP:Port");
    }


    std::string transmittedString () {return sent_string_;}
    void setStringToBeRead (std::string const& read_string) {string_to_return_on_read_ = read_string;}
    void reset () {sent_string_.clear (); string_to_return_on_read_.clear ();}

    void enableFailureIfSending (bool enabled) {failure_if_sending_ = enabled;}
private:
    boost::condition_variable wait_for_data_condition_;
    boost::mutex transmitted_string_lock_;

    std::string sent_string_;
    std::string string_to_return_on_read_;
    bool failure_if_sending_;
};


//-----------------------------------------------------------------------------
/// TestTCPServerSocket
///
/// test implementation of the TCPServerSocket class
class TestTCPServerSocket : public tia::TCPServerSocket
{
private:
    typedef std::list<boost::thread*> ThreadList;
public:
    TestTCPServerSocket () : listener_ (0) {}

    //-------------------------------------------------------------------------
    virtual void startListening (unsigned /*port*/,
                                 tia::NewConnectionListener* new_connection_listener)
    {
        listener_ = new_connection_listener;
    }

    //-------------------------------------------------------------------------
    virtual unsigned startListening (tia::NewConnectionListener* new_connection_listener)
    {
        listener_ = new_connection_listener;
        return 0;
    }

    //-------------------------------------------------------------------------
    virtual void stopListening ()
    {
        listener_ = 0;
    }

    //-------------------------------------------------------------------------
    void shutdown ()
    {
        for (ThreadList::iterator thread_iter = server_threads_.begin();
             thread_iter != server_threads_.end(); ++thread_iter)
        {
            if ((*thread_iter)->joinable ())
                (*thread_iter)->join ();
            delete *thread_iter;
        }
    }

    //-------------------------------------------------------------------------
    boost::shared_ptr<TestSocket> clientConnects ()
    {
        boost::shared_ptr<TestSocket> server_to_client_socket (new TestSocket);
        if (listener_)
        {
            boost::thread* new_server_thread = new boost::thread (boost::bind (&tia::NewConnectionListener::newConnection, listener_, server_to_client_socket));
            server_threads_.push_back (new_server_thread);
        }

        return server_to_client_socket;
    }

private:
    ThreadList server_threads_;
    tia::NewConnectionListener* listener_;
};


#endif // TEST_SOCKET_H
