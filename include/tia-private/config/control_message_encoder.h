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

namespace tia
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
