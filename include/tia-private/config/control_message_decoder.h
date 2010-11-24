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
* @control_message_decoder.h
*
* @brief \TODO.
*
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
#include "config/control_messages.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

class ControlMsgDecoder
{
public:
  virtual ~ControlMsgDecoder() {  };
  ///
  void setInputStream(std::istream* stream) { istream_ = stream; }
  ///
  virtual ControlMsg* decodeMsg() = 0;
  ///
  virtual void decodeMsg(KeepAliveMsg& msg) = 0;
  ///
  virtual void decodeMsg(GetConfigMsg& msg) = 0;
  ///
  virtual void decodeMsg(GetDataConnectionMsg& msg) = 0;
  ///
  virtual void decodeMsg(DataConnectionMsg& msg) = 0;
  ///
  virtual void decodeMsg(StartTransmissionMsg& msg) = 0;
  ///
  virtual void decodeMsg(StopTransmissionMsg& msg) = 0;
  ///
  virtual void decodeMsg(ConfigMsg& msg) = 0;
  ///
  virtual void decodeMsg(ReplyMsg& msg) = 0;

protected:
  std::istream* istream_;
};

//-----------------------------------------------------------------------------

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
  ///
  bool getXmlMsgType(ticpp::Element& header, std::string& type) const;
  ///
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
