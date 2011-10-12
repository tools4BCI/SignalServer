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

namespace tia
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
