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

#ifndef TID_CLIENT_H
#define TID_CLIENT_H

#include "tid_client_base.h"
#include "tid_shm_client.h"

#include <boost/thread.hpp>

//-----------------------------------------------------------------------------

namespace TiD
{

class TiDClient : public TiDClientBase, public TiDSHMClient
{
  friend class TimedTiDClient;
  public:
    TiDClient();
    virtual ~TiDClient();

    virtual void startReceiving( bool throw_on_error = 0 );
    virtual void stopReceiving();

    void sendMessage(std::string& tid_xml_context);
    void sendMessage(IDMessage& msg);

    bool newMessagesAvailable();
    void getLastMessagesContexts( std::vector< IDMessage >& messages  );
    void clearMessages();

  private:

    virtual void receiveSHMFinalHook()
    {

    }

    boost::thread*                                    receive_thread_;
    boost::thread*                                    io_service_thread_;
    boost::thread*                                    io_service_thread_2_;

};

//-----------------------------------------------------------------------------

}  //TiD

#endif // TID_CLIENT_H
