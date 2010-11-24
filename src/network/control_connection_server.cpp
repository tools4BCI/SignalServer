/*
    This file is part of TOBI Interface A (TiA).

    TOBI Interface A (TiA) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface A (TiA) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface A (TiA).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @control_connection_server.cpp
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
#include "tia/signal_server.h"

#include "tia-private/network/control_connection.h"
#include "tia-private/network/control_connection_server.h"
#include "tia-private/network/tcp_data_server.h"
#include "tia-private/network/udp_data_server.h"

namespace tobiss
{

using std::vector;
using std::string;
using std::map;
using std::cout;
using std::endl;

using boost::uint16_t;
using boost::uint32_t;

//-----------------------------------------------------------------------------

ControlConnectionServer::ControlConnectionServer(std::map<std::string,std::string> subject_info,
                                                 boost::asio::io_service& io_service,
                                                 SignalServer& server)
  : TCPServer(io_service),
  server_(server),
  subject_info_(0),
  signal_info_(0)
{
  signal_info_ = new SignalInfo;
  subject_info_ = new SubjectInfo;

  createSubjectInfo(subject_info);
  createSignalInfo();
}

//-----------------------------------------------------------------------------

ControlConnectionServer::~ControlConnectionServer()
{
  delete signal_info_;
  delete subject_info_;
}

//-----------------------------------------------------------------------------

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

    signal_info_->signals().insert(make_pair(sig_str_type,signal));
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

  // lock the connection list
  boost::unique_lock<boost::mutex> lock(mutex_);

  int local_port = new_connection->socket().local_endpoint().port();

  ControlConnection::ConnectionID id = make_pair(local_port, TCPConnection::endpointToString(
                              new_connection->socket().remote_endpoint()));

  ControlConnection::pointer connection = ControlConnection::create(io_service_, id,
                                                                    *this, new_connection);

  cout << " Client @" << id.second << " has connected." <<  endl;


  CtrlConnHandlers::iterator it = connections_.insert(make_pair(id, connection)).first;

  cout << " # Connected clients: " << connections_.size() << endl;

  connection->start();

  startAccept();
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

} // Namespace tobiss

// End Of File
