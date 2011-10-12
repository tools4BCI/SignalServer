/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

/**
* @file control_connection_server.cpp
*
* @brief \TODO.
*
**/

// Boost
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/iostreams/stream.hpp>

// local
#include "tia/constants.h"
#include "tia/tia_server.h"

#include "tia-private/network/control_connection.h"
#include "tia-private/network/control_connection_server.h"
#include "tia-private/network/tcp_data_server.h"
#include "tia-private/network/udp_data_server.h"

//#include "tia-private/newtia/network_impl/boost_tcp_socket_impl.h"
//#include "tia-private/newtia/server_impl/control_connection_2.h"
//#include "tia-private/newtia/server_impl/fusty_data_server_impl.h"
//#include "tia-private/newtia/fusty_hardware_interface_impl.h"

namespace tia
{

using std::vector;
using std::string;
using std::map;
using std::cout;
using std::endl;

using boost::uint16_t;
using boost::uint32_t;

static const int SECONDS_TO_RE_CHECK_CONNECTIONS = 1;

//-----------------------------------------------------------------------------

ControlConnectionServer::ControlConnectionServer(std::map<std::string,std::string> subject_info,
                                                 boost::asio::io_service& io_service,
                                                 TiAServer& server)
  : TCPServer(io_service),
  server_(server),
  subject_info_(0),
  signal_info_(0)
//  data_server_ (0),
//  hardware_interface_ (0),
//  check_connections_timer_ (io_service)
{
  signal_info_ = new SignalInfo;
  subject_info_ = new SubjectInfo;
//  hardware_interface_ = new tia::FustyHardwareInterfaceImpl (*this);

  createSubjectInfo(subject_info);
  createSignalInfo();
}

//-----------------------------------------------------------------------------

ControlConnectionServer::~ControlConnectionServer()
{
  delete signal_info_;
  delete subject_info_;
//  delete hardware_interface_;
//  delete data_server_;
//  for (std::map<unsigned, tia::ControlConnection2*>::iterator iter = new_connections_.begin();
//       iter != new_connections_.end(); ++iter)
//  {
//    delete iter->second;
//  }
//  for (std::map<unsigned, tia::Socket*>::iterator iter = new_sockets_.begin();
//       iter != new_sockets_.end(); ++iter)
//  {
//    delete iter->second;
//  }
}

////-----------------------------------------------------------------------------

TCPDataServer* ControlConnectionServer::tcpDataServer() const
{
  return server_.tcp_data_server_;
}

//-----------------------------------------------------------------------------

UDPDataServer* ControlConnectionServer::udpDataServer() const
{
  return server_.udp_data_server_;
}

//-----------------------------------------------------------------------------

void ControlConnectionServer::getConfig(ConfigMsg& config)
{
  config.subject_info = *subject_info_;
  config.signal_info  = *signal_info_;
}

//-----------------------------------------------------------------------------

void ControlConnectionServer::createSubjectInfo(std::map<std::string,std::string> subject_map)
{
//  map<string,string> subject_map = server_.config_->parseSubject();

  subject_info_->setId(subject_map["id"]);
  subject_info_->setFirstName(subject_map["first_name"]);
  subject_info_->setSurname(subject_map["surname"]);
  subject_info_->setBirthday(subject_map["birthday"]);
  subject_info_->setMedication(subject_map["medication"]);

  std::string value = subject_map["sex"];
  if (value == "m")
    subject_info_->setSex(SubjectInfo::Male);
  else if (value == "f")
    subject_info_->setSex(SubjectInfo::Female);

  value = subject_map["handedness"];
  if (value == "r")
    subject_info_->setHandedness(SubjectInfo::RightHanded);
  else if (value == "l")
    subject_info_->setHandedness(SubjectInfo::LeftHanded);

  value = subject_map["glasses"];
  if (value == "y")
    subject_info_->setShortInfo(SubjectInfo::Glasses, SubjectInfo::Yes);
  else if (value == "n")
    subject_info_->setShortInfo(SubjectInfo::Glasses, SubjectInfo::No);
  else
    subject_info_->setShortInfo(SubjectInfo::Glasses, SubjectInfo::Unknown);

  value = subject_map["smoker"];
  if (value == "y")
    subject_info_->setShortInfo(SubjectInfo::Smoking, SubjectInfo::Yes);
  else if (value == "n")
    subject_info_->setShortInfo(SubjectInfo::Smoking, SubjectInfo::No);
  else
    subject_info_->setShortInfo(SubjectInfo::Smoking, SubjectInfo::Unknown);
}

//-----------------------------------------------------------------------------

void ControlConnectionServer::createSignalInfo()
{
  const std::vector<uint32_t>& sig_types       = server_.sig_types_;
  const std::vector<uint16_t>& blocksizes      = server_.blocksizes_;
  const std::vector<uint32_t>& fs_per_sig_type = server_.fs_per_sig_type_;

  const std::map<uint32_t, std::vector<std::string> >& channel_map = server_.channels_per_sig_type_;

  assert(sig_types.size() == blocksizes.size() && sig_types.size() == fs_per_sig_type.size());

  cout << endl;
  cout << " Sent Signal Types: (ordered)" << endl;

  for (vector<uint32_t>::size_type index = 0; index < sig_types.size(); ++index)
  {
    Signal signal;

    uint32_t sig_num_type = sig_types[index];
    std::string sig_str_type = Constants().getSignalName(sig_num_type);
    signal.setType(sig_str_type);
    cout << "   ... Signal type " << sig_str_type << endl;

    uint16_t block_size = blocksizes[index];
    signal.setBlockSize(block_size);

    uint32_t fs = fs_per_sig_type[index];
    signal.setSamplingRate(fs);

    std::map<uint32_t, std::vector<std::string> >::const_iterator it_channel_map =
      channel_map.find(sig_num_type);

    if (it_channel_map != channel_map.end())
    {
      const std::vector<std::string>& channel_names = (*it_channel_map).second;
      std::vector<std::string>::const_iterator it_channels = channel_names.begin();
      std::vector<std::string>::const_iterator end_channels = channel_names.end();
      for (; it_channels != end_channels; ++it_channels)
      {
        Channel channel;
        channel.setId(*it_channels);
        signal.channels().push_back(channel);
      }
    }

    signal_info_->signals().insert(make_pair(sig_str_type, signal));
  }

  signal_info_->setMasterBlockSize(server_.master_blocksize_);
  signal_info_->setMasterSamplingRate(server_.master_samplingrate_);
}


//-----------------------------------------------------------------------------

void ControlConnectionServer::handleAccept(const TCPConnection::pointer& new_connection,
      const boost::system::error_code& error)
{
  if (error)
  {
    // TODO: error handling
    return;
  }

//  if (!data_server_)
//    data_server_ = new tia::FustyDataServerImpl (*(server_.tcp_data_server_), *(server_.udp_data_server_));

//  {
//    boost::system::error_code err;
//    checkConnections (err);
//  }

  // lock the connection list
  boost::unique_lock<boost::mutex> lock(mutex_);

  unsigned short local_port = new_connection->socket().local_endpoint().port();

//  if (server_.new_tia_)
//  {
//    tia::Socket* new_socket = new tia::BoostTCPSocketImpl (new_connection);
//    tia::ControlConnection2* new_control_connection = new tia::ControlConnection2 (*new_socket, *data_server_, *hardware_interface_, *(server_.server_state_server_));
//    unsigned id = new_control_connection->getId();
//    new_connections_[id] = new_control_connection;
//    new_sockets_[id] = new_socket;
//    cout << " Client " << id <<" @" << new_connection->socket().remote_endpoint() << " has connected. (local: "  << new_connection->socket().local_endpoint() << ")"<< endl;
//    cout << " # Connected clients: " << new_connections_.size () << endl;
//    new_control_connection->asyncStart ();
//  }
//  else
//  {
    ControlConnection::ConnectionID id = make_pair(local_port, TCPConnection::endpointToString(
                                new_connection->socket().remote_endpoint()));

    ControlConnection::pointer connection = ControlConnection::create(io_service_, id,
                                                                    *this, new_connection);

    cout << " Client @" << id.second << " has connected." <<  endl;


    connections_.insert(make_pair(id, connection)).first;

    cout << " # Connected clients: " << connections_.size() << endl;

    connection->start();
//  }
  startAccept ();
}

//-----------------------------------------------------------------------------

void ControlConnectionServer::clientHasDisconnected(const ControlConnection::ConnectionID& id)
{
  boost::unique_lock<boost::mutex> lock(mutex_);

  cout << " Connection to client @" << id.second << " has been closed." << endl;
  connections_.erase(id);

  cout << " # Connected clients: " << connections_.size() << endl;
}

//-----------------------------------------------------------------------------
//void ControlConnectionServer::checkConnections (boost::system::error_code error)
//{
//  if (error)
//      return;
//  boost::unique_lock<boost::mutex> lock(mutex_);
//  #ifdef DEBUG
//    static unsigned call_counter = 0;
//    cout << " <- check connection "<< ++call_counter <<" -> " << endl;
//  #endif
//  std::list<unsigned> to_be_removed;
//  for (std::map<unsigned, tia::ControlConnection2*>::iterator iter = new_connections_.begin();
//       iter != new_connections_.end(); ++iter)
//  {
//    if (!(iter->second->isRunning()))
//      to_be_removed.push_back (iter->first);
//  }

//  for (std::list<unsigned>::iterator iter = to_be_removed.begin();
//       iter != to_be_removed.end(); ++iter)
//  {
//    cout << "  -- Removing connection to: " << new_sockets_[*iter]->getRemoteEndPointAsString() << endl;
//    delete new_connections_[*iter];
//    delete new_sockets_[*iter];
//    new_connections_.erase (*iter);
//    new_sockets_.erase (*iter);
//  }
//  if (to_be_removed.size())
//      cout << " # Connected clients: " << new_connections_.size() << endl;

//  check_connections_timer_.cancel ();
//  check_connections_timer_.expires_from_now (boost::posix_time::seconds (SECONDS_TO_RE_CHECK_CONNECTIONS));
//  check_connections_timer_.async_wait(boost::bind(&ControlConnectionServer::checkConnections, this, boost::asio::placeholders::error));
//}


//-----------------------------------------------------------------------------

} // Namespace tobiss

// End Of File
