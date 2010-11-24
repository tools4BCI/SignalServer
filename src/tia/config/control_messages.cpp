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
* @control_messages.cpp
*
* @brief \TODO.
*
**/

// local
#include "tia-private/config/control_message_decoder.h"
#include "tia-private/config/control_message_encoder.h"
#include "tia-private/config/control_messages.h"

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
