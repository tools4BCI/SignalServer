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
* @file control_messages.cpp
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
