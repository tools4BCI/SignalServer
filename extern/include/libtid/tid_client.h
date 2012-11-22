#ifndef TID_CLIENT_H
#define TID_CLIENT_H

#include "tid_client_base.h"

#include <boost/thread.hpp>

//-----------------------------------------------------------------------------

namespace TiD
{

class TiDClient : public TiDClientBase
{
  public:
    TiDClient();
    virtual ~TiDClient();

    virtual void startReceiving( bool throw_on_error = 0 );
    virtual void stopReceiving();

  private:
    boost::thread*                                    receive_thread_;
    boost::thread*                                    io_service_thread_;
    boost::thread*                                    io_service_thread_2_;

};

//-----------------------------------------------------------------------------

}  //TiD

#endif // TID_CLIENT_H
