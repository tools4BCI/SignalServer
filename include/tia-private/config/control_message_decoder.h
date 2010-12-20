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
* @file control_message_decoder.h
* @brief This file includes an interface and a concrete class to decode control messages.
**/

#ifndef CONTROLMESSAGEDECODER_H
#define CONTROLMESSAGEDECODER_H

// Standard
#include <assert.h>
#include <iostream>

// boost
#include <boost/thread/condition.hpp>

// TICPP
#include "ticpp/ticpp.h"

// local
#include "control_messages.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class ControlMsgDecoder
* @brief Interface to decode control message
*
* @todo Check constructor
* @todo Find out what decodeMsg() is exactly planned/used for
*/
class ControlMsgDecoder
{
public:
  virtual ~ControlMsgDecoder() {  };

  ///< Set the istream to decode control messages from.
  void setInputStream(std::istream* stream) { istream_ = stream; }

  ///< Decode message already in the istream.
  virtual ControlMsg* decodeMsg() = 0;

  ///< Decode a KeepAlive message
  virtual void decodeMsg(KeepAliveMsg& msg) = 0;

  ///< Decode a GetConfig message
  virtual void decodeMsg(GetConfigMsg& msg) = 0;

  ///< Decode a GetDataCnnection message
  virtual void decodeMsg(GetDataConnectionMsg& msg) = 0;

  ///< Decode a DataConnection message
  virtual void decodeMsg(DataConnectionMsg& msg) = 0;

  ///< Decode a StartTransmission message
  virtual void decodeMsg(StartTransmissionMsg& msg) = 0;

  ///< Decode a StopTransmission message
  virtual void decodeMsg(StopTransmissionMsg& msg) = 0;

  ///< Decode a Config message
  virtual void decodeMsg(ConfigMsg& msg) = 0;

  ///< Decode a Reply message
  virtual void decodeMsg(ReplyMsg& msg) = 0;

protected:
  std::istream* istream_;    ///< The istream to decode messages from
};

//-----------------------------------------------------------------------------

/**
* @class ControlMsgEncoderXML
* @brief Concrete class to decode XML control messages
* @todo  Find out what "decodeHeader" is exactly planned/used for.
*/
class ControlMsgDecoderXML : public ControlMsgDecoder
{
public:
  ///
  ControlMsgDecoderXML();
  ///
  virtual ~ControlMsgDecoderXML();
  ///
  virtual ControlMsg* decodeMsg();
  ///
  virtual void decodeMsg(KeepAliveMsg& msg);
  ///
  virtual void decodeMsg(GetConfigMsg& msg);
  ///
  virtual void decodeMsg(GetDataConnectionMsg& msg);
  ///
  virtual void decodeMsg(DataConnectionMsg& msg);
  ///
  virtual void decodeMsg(StartTransmissionMsg& msg);
  ///
  virtual void decodeMsg(StopTransmissionMsg& msg);
  ///
  virtual void decodeMsg(ConfigMsg& msg);
  ///
  virtual void decodeMsg(ReplyMsg& msg);

private:
  ///
  ControlMsg* decodeMsgInternal();
  ///< Write the type of the XML message into "type".
  bool getXmlMsgType(ticpp::Element& header, std::string& type) const;
  ///< Get the sender from the control message
  bool decodeHeader(ControlMsg& msg);

private:
  TiXmlDocument*                        tixml_doc_;
  ticpp::Document*                      xml_doc_;
  ticpp::Element*                       xml_msg_header_;

  typedef boost::shared_ptr<ControlMsg> ControlMsgHandle;
  std::map<std::string, ControlMsgHandle> prototypes_;
};

} // Namespace tobiss

#endif //CONTROLMESSAGEDECODER_H

// End Of File
