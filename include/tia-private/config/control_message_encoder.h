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
* @file control_message_encoder.h
*
* @brief This class includes an interface and a concrete class to encode control messages.
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

/**
* @class ControlMsgEncoder
* @brief Interface to encode control message
*/
class ControlMsgEncoder
{
public:
  /**
  * @brief Constuctor
  */
  ControlMsgEncoder() {}
  /**
  * @brief Destuctor
  */
  virtual ~ControlMsgEncoder(){}

  ///< Write a KeepAlive message to the ostream
  virtual void encodeMsg(const KeepAliveMsg& msg, std::ostream& stream) = 0;

  ///< Write a GetConfig message to the ostream
  virtual void encodeMsg(const GetConfigMsg& msg, std::ostream& stream) = 0;

  ///< Write a GetDataConnection message to the ostream
  virtual void encodeMsg(const GetDataConnectionMsg& msg, std::ostream& stream) = 0;

  ///< Write a DataCnnection message to the ostream
  virtual void encodeMsg(const DataConnectionMsg& msg, std::ostream& stream) = 0;

  ///< Write a StartTransmission message to the ostream
  virtual void encodeMsg(const StartTransmissionMsg& msg, std::ostream& stream) = 0;

  ///< Write a StopTransmission message to the ostream
  virtual void encodeMsg(const StopTransmissionMsg& msg, std::ostream& stream) = 0;

  ///< Write a Config message to the ostream
  virtual void encodeMsg(const ConfigMsg& msg, std::ostream& stream) = 0;

  ///< Write a Reply message to the ostream
  virtual void encodeMsg(const ReplyMsg& msg, std::ostream& stream) = 0;
};

//-----------------------------------------------------------------------------

/**
* @class ControlMsgEncoderXML
* @brief Concrete class to encode XML control message
*/
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
  /// Encode a base control message
  void encodeBaseMsg(const ControlMsg& msg, const std::string& xml_msg_type,
      TiXmlDocument& doc, TiXmlElement*& xml_msg);

  /// Write the XML control message to the ostream
  void writeXMLMsg(TiXmlDocument& doc, std::ostream& stream);
};

//-----------------------------------------------------------------------------

} // namespace tobiss

#endif //CONTROLMESSAGEENCODER_H

// End Of File
