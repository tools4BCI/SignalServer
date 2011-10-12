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
* @file control_messages.h
*
* @brief \TODO.
*
**/

#ifndef CONTROLMESSAGES_H
#define CONTROLMESSAGES_H

// Standard
#include <assert.h>
#include <iostream>

// boost
#include <boost/cstdint.hpp>

// local
#include "tia/ss_meta_info.h"

namespace tia
{

// forward declarations
class ControlMsgEncoder;
class ControlMsgDecoder;

static const std::string MESSAGE_VERSION("0.1");

//-----------------------------------------------------------------------------

class ControlMsg
{
  public:
    enum MsgType {
      KeepAlive = 0,
      GetConfig,
      Config,
      GetDataConnection,
      DataConnection,
      StartTransmission,
      StopTransmission,
      OkReply,
      ErrorReply
    };

    ///
    virtual ~ControlMsg(){}
    ///
    MsgType msgType() const { return msg_type_; }

    ///
    void setSender(const std::string& sender) { sender_ = sender; }
    ///
    std::string sender() const { return sender_; }
    ///
    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const = 0;
    ///
    virtual void readMsg(ControlMsgDecoder& decoder) = 0;
    ///
    virtual ControlMsg* clone() const = 0;

  protected:
    ///
    ControlMsg(MsgType msg_type) : msg_type_(msg_type) {}

  private:
    MsgType     msg_type_;   ///<
    std::string sender_;     ///<
};

//-----------------------------------------------------------------------------

class KeepAliveMsg : public ControlMsg
{
  public:
    KeepAliveMsg() : ControlMsg(ControlMsg::KeepAlive)  {}
    virtual ~KeepAliveMsg(){}

    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const;

    virtual void readMsg(ControlMsgDecoder& decoder);

    virtual ControlMsg* clone() const { return new KeepAliveMsg(); };
};

//-----------------------------------------------------------------------------

class GetConfigMsg : public ControlMsg
{
  public:
    GetConfigMsg() : ControlMsg(ControlMsg::GetConfig) {}
    virtual ~GetConfigMsg(){}

    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const;

    virtual void readMsg(ControlMsgDecoder& decoder);

    virtual ControlMsg* clone() const { return new GetConfigMsg(); };
};

//-----------------------------------------------------------------------------

class StopTransmissionMsg : public ControlMsg
{
  public:
    StopTransmissionMsg() : ControlMsg(ControlMsg::StopTransmission) {}
    virtual ~StopTransmissionMsg(){}

    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const;

    virtual void readMsg(ControlMsgDecoder& decoder);

    virtual ControlMsg* clone() const { return new StopTransmissionMsg(); };
};

//-----------------------------------------------------------------------------

class GetDataConnectionMsg : public ControlMsg
{
  public:
    enum ConnectionType {
      Udp = 1,
      Tcp
    };

  public:
    GetDataConnectionMsg() : ControlMsg(ControlMsg::GetDataConnection), connection_type_(Tcp)  {}
    virtual ~GetDataConnectionMsg(){}

    int connectionType() const { return connection_type_; }
    void setConnectionType(int type) { connection_type_ = type; }

    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const;

    virtual void readMsg(ControlMsgDecoder& decoder);

    virtual ControlMsg* clone() const { return new GetDataConnectionMsg(); };

  private:
    int connection_type_;
};

//-----------------------------------------------------------------------------

class DataConnectionMsg : public ControlMsg
{
  public:
    DataConnectionMsg() : ControlMsg(ControlMsg::DataConnection)
    {}

    ///
    virtual ~DataConnectionMsg(){}

    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const;

    virtual void readMsg(ControlMsgDecoder& decoder);

    virtual ControlMsg* clone() const { return new DataConnectionMsg(); };

    std::string address() const { return address_; }
    void setAddress(const std::string& addr){ address_ = addr; }

    boost::uint16_t port() const { return port_; }
    void setPort(boost::uint16_t port) { port_ = port; }

  private:
    std::string   address_;
    boost::uint16_t      port_;
};

//-----------------------------------------------------------------------------

class StartTransmissionMsg : public ControlMsg
{
  public:
    StartTransmissionMsg() : ControlMsg(ControlMsg::StartTransmission) {}
    virtual ~StartTransmissionMsg(){}

    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const;

    virtual void readMsg(ControlMsgDecoder& decoder);

    virtual ControlMsg* clone() const { return new StartTransmissionMsg(); };
};

//-----------------------------------------------------------------------------

class ConfigMsg : public ControlMsg
{
  public:
    ConfigMsg();

    virtual ~ConfigMsg();

    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const;

    virtual void readMsg(ControlMsgDecoder& decoder);

    virtual ControlMsg* clone() const { return new ConfigMsg(); };

  public:
    SubjectInfo subject_info;
    SignalInfo  signal_info;
};

//-----------------------------------------------------------------------------
///
class ReplyMsg : public ControlMsg
{
  public:
    static ReplyMsg error() { return ReplyMsg(ErrorReply); }
    static ReplyMsg ok()    { return ReplyMsg(OkReply); }

    ///
    virtual ~ReplyMsg(){}

    virtual void writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const;

    virtual void readMsg(ControlMsgDecoder& decoder);

    virtual ControlMsg* clone() const { return new ReplyMsg(msgType()); };

  protected:
    ReplyMsg(MsgType type) : ControlMsg(type)
    {}
};

} // Namespace tobiss

#endif //CONTROLMESSAGES_H

// End Of File
