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
* @file control_connection_server.h
*
* @brief \TODO.
*
**/

#ifndef CONTROLMESSAGEENCODER_H
#define CONTROLMESSAGEENCODER_H

// Standard
#include <assert.h>
#include <iostream>

// boost
#include <boost/cstdint.hpp>

// local external
#include "ticpp/tinyxml.h"

// local
#include "control_messages.h"

namespace tobiss
{
//-----------------------------------------------------------------------------

class ControlMsgEncoder
{
public:
  ControlMsgEncoder() {}
  virtual ~ControlMsgEncoder(){}

  ///
  virtual void encodeMsg(const KeepAliveMsg& msg, std::ostream& stream) = 0;
  ///
  virtual void encodeMsg(const GetConfigMsg& msg, std::ostream& stream) = 0;
  ///
  virtual void encodeMsg(const GetDataConnectionMsg& msg, std::ostream& stream) = 0;
  ///
  virtual void encodeMsg(const DataConnectionMsg& msg, std::ostream& stream) = 0;
  ///
  virtual void encodeMsg(const StartTransmissionMsg& msg, std::ostream& stream) = 0;
  ///
  virtual void encodeMsg(const StopTransmissionMsg& msg, std::ostream& stream) = 0;
  ///
  virtual void encodeMsg(const ConfigMsg& msg, std::ostream& stream) = 0;
  ///
  virtual void encodeMsg(const ReplyMsg& msg, std::ostream& stream) = 0;
};

//-----------------------------------------------------------------------------

class ControlMsgEncoderXML : public ControlMsgEncoder
{
public:
  ///
  virtual void encodeMsg(const KeepAliveMsg& msg, std::ostream& stream);
  ///
  virtual void encodeMsg(const GetConfigMsg& msg, std::ostream& stream);
  ///
  virtual void encodeMsg(const GetDataConnectionMsg& msg, std::ostream& stream);
  ///
  virtual void encodeMsg(const DataConnectionMsg& msg, std::ostream& stream);
  ///
  virtual void encodeMsg(const StartTransmissionMsg& msg, std::ostream& stream);
  ///
  virtual void encodeMsg(const StopTransmissionMsg& msg, std::ostream& stream);
  ///
  virtual void encodeMsg(const ConfigMsg& msg, std::ostream& stream);
  ///
  virtual void encodeMsg(const ReplyMsg& msg, std::ostream& stream);

protected:
  void encodeBaseMsg(const ControlMsg& msg, const std::string& xml_msg_type,
      TiXmlDocument& doc, TiXmlElement*& xml_msg);

  void writeXMLMsg(TiXmlDocument& doc, std::ostream& stream);
};

//-----------------------------------------------------------------------------

} // namespace tobiss

#endif //CONTROLMESSAGEENCODER_H

// End Of File
