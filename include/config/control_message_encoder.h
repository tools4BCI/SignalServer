/**
* @control_connection_server.h
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

// local
#include "config/control_messages.h"

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
