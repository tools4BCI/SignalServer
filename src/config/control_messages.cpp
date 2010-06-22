/**
* @control_messages.cpp
*
* @brief \TODO.
*
**/

// local
#include "config/control_message_decoder.h"
#include "config/control_message_encoder.h"
#include "config/control_messages.h"

namespace tobiss
{

//-----------------------------------------------------------------------------

void KeepAliveMsg::writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const
{
  encoder.encodeMsg(*this, stream);
}

//-----------------------------------------------------------------------------

void KeepAliveMsg::readMsg(ControlMsgDecoder& decoder)
{
  decoder.decodeMsg(*this);
}

//-----------------------------------------------------------------------------

void GetConfigMsg::writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const
{
  encoder.encodeMsg(*this, stream);
}

//-----------------------------------------------------------------------------

void GetConfigMsg::readMsg(ControlMsgDecoder& decoder)
{
  decoder.decodeMsg(*this);
}

//-----------------------------------------------------------------------------

void StopTransmissionMsg::writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const
{
  encoder.encodeMsg(*this, stream);
}

//-----------------------------------------------------------------------------

void StopTransmissionMsg::readMsg(ControlMsgDecoder& decoder)
{
  decoder.decodeMsg(*this);
}

//-----------------------------------------------------------------------------

void StartTransmissionMsg::writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const
{
  encoder.encodeMsg(*this, stream);
}

//-----------------------------------------------------------------------------

void StartTransmissionMsg::readMsg(ControlMsgDecoder& decoder)
{
  decoder.decodeMsg(*this);
}

//-----------------------------------------------------------------------------

void ReplyMsg::writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const
{
  encoder.encodeMsg(*this, stream);
}

//-----------------------------------------------------------------------------

void ReplyMsg::readMsg(ControlMsgDecoder& decoder)
{
  decoder.decodeMsg(*this);
}

//-----------------------------------------------------------------------------

void GetDataConnectionMsg::writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const
{
  encoder.encodeMsg(*this, stream);
}

//-----------------------------------------------------------------------------

void GetDataConnectionMsg::readMsg(ControlMsgDecoder& decoder)
{
  decoder.decodeMsg(*this);
}

//-----------------------------------------------------------------------------

void DataConnectionMsg::writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const
{
  encoder.encodeMsg(*this, stream);
}

//-----------------------------------------------------------------------------

void DataConnectionMsg::readMsg(ControlMsgDecoder& decoder)
{
  decoder.decodeMsg(*this);
}


//-----------------------------------------------------------------------------

ConfigMsg::ConfigMsg() :
  ControlMsg(Config)
{}

//-----------------------------------------------------------------------------

ConfigMsg::~ConfigMsg()
{}

//-----------------------------------------------------------------------------

void ConfigMsg::writeMsg(ControlMsgEncoder& encoder, std::ostream& stream) const
{
  encoder.encodeMsg(*this, stream);
}

//-----------------------------------------------------------------------------

void ConfigMsg::readMsg(ControlMsgDecoder& decoder)
{
  decoder.decodeMsg(*this);
}

//-----------------------------------------------------------------------------

} // Namespace tobiss

// End Of File
