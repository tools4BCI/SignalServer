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
* @file control_message_encoder.cpp
*
* @brief \TODO.
*
**/

// Boost
#include <boost/lexical_cast.hpp>

// local
#include "tia/constants.h"

#include "tia-private/config/control_message_encoder.h"

namespace tobiss
{

using boost::lexical_cast;
using boost::bad_lexical_cast;

using std::cout;
using std::endl;
using std::string;
using std::stringstream;


//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const KeepAliveMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg,"alive", doc, xml_msg);
  assert(xml_msg != 0);

  TiXmlElement* cmd = new TiXmlElement("alive");
  xml_msg->LinkEndChild(cmd);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const GetConfigMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg, "getConfig", doc, xml_msg);
  assert(xml_msg != 0);

  TiXmlElement* cmd = new TiXmlElement("getConfig");
  xml_msg->LinkEndChild(cmd);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const StopTransmissionMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg,"stopTransmission", doc, xml_msg);
  assert(xml_msg != 0);

  TiXmlElement* cmd = new TiXmlElement("stopTransmission");
  xml_msg->LinkEndChild(cmd);
  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const GetDataConnectionMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg,"getDataConnection", doc, xml_msg);
  assert(xml_msg != 0);

  TiXmlElement* cmd_element = new TiXmlElement("getDataConnection");
  TiXmlElement* element = new TiXmlElement("connectionType");

  element->LinkEndChild(new TiXmlText(
      msg.connectionType() == GetDataConnectionMsg::Tcp ? "tcp" : "udp"));
  cmd_element->LinkEndChild(element);
  xml_msg->LinkEndChild(cmd_element);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const DataConnectionMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg,"dataConnection", doc, xml_msg);
  assert(xml_msg != 0);

  TiXmlElement* reply_element = new TiXmlElement("dataConnection");

  TiXmlElement* element = 0;

  if (!msg.address().empty())
  {
    element = new TiXmlElement("address");
    element->LinkEndChild(new TiXmlText(msg.address()));
    reply_element->LinkEndChild(element);
  }

  element = new TiXmlElement("port");
  string value = lexical_cast<string>(msg.port());
  element->LinkEndChild(new TiXmlText(value));
  reply_element->LinkEndChild(element);

  xml_msg->LinkEndChild(reply_element);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const StartTransmissionMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg,"startTransmission", doc, xml_msg);
  assert(xml_msg != 0);

  TiXmlElement* cmd_element = new TiXmlElement("startTransmission");
  xml_msg->LinkEndChild(cmd_element);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const ConfigMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg,"config", doc, xml_msg);
  assert(xml_msg != 0);

  TiXmlElement* config = new TiXmlElement("config");

  xml_msg->LinkEndChild(config);

  const SubjectInfo& subject = msg.subject_info;

  TiXmlElement* subject_elem = new TiXmlElement("subject");

  TiXmlElement* element = 0;

  element = new TiXmlElement("id");
  element->LinkEndChild(new TiXmlText(subject.id()));
  subject_elem->LinkEndChild(element);

  element = new TiXmlElement("firstName");
  element->LinkEndChild(new TiXmlText(subject.firstName()));
  subject_elem->LinkEndChild(element);

  element = new TiXmlElement("surname");
  element->LinkEndChild(new TiXmlText(subject.surname()));
  subject_elem->LinkEndChild(element);

  element = new TiXmlElement("sex");
  element->LinkEndChild(new TiXmlText(subject.sex() == SubjectInfo::Male ? "m" : "f"));
  subject_elem->LinkEndChild(element);

  element = new TiXmlElement("birthday");
  element->LinkEndChild(new TiXmlText(subject.birthday()));
  subject_elem->LinkEndChild(element);

  element = new TiXmlElement("handedness");
  element->LinkEndChild(new TiXmlText(subject.handedness() == SubjectInfo::RightHanded ? "r" : "l"));
  subject_elem->LinkEndChild(element);

  element = new TiXmlElement("medication");
  element->LinkEndChild(new TiXmlText(subject.medication()));
  subject_elem->LinkEndChild(element);

  SubjectInfo::ShortInfoMap::const_iterator it_short_infos = subject.shortInfoMap().begin();
  SubjectInfo::ShortInfoMap::const_iterator end_short_infos = subject.shortInfoMap().end();

  for (; it_short_infos != end_short_infos; ++it_short_infos)
  {
    SubjectInfo::ShortInfoType type  = (*it_short_infos).first;
    SubjectInfo::ShortInfoValue value = (*it_short_infos).second;

    std::string value_as_str = value == SubjectInfo::Yes ? "yes" : "no";
    switch(type)
    {
      case SubjectInfo::Glasses:
        element = new TiXmlElement("glasses");
        element->LinkEndChild(new TiXmlText(value_as_str));
        subject_elem->LinkEndChild(element);
        break;
      case SubjectInfo::Smoking:
        element = new TiXmlElement("smoker");
        element->LinkEndChild(new TiXmlText(value_as_str));
        subject_elem->LinkEndChild(element);
        break;
    }
  }

  config->LinkEndChild(subject_elem);

  const SignalInfo& signal_info = msg.signal_info;

  TiXmlElement* signal_info_elem = new TiXmlElement("signalInfo");

  TiXmlElement* master_elem = new TiXmlElement("master");

  element = new TiXmlElement("blockSize");
  string value = lexical_cast<string>(signal_info.masterBlockSize());
  element->LinkEndChild(new TiXmlText(value));
  master_elem->LinkEndChild(element);

  element = new TiXmlElement("samplingRate");
  value = lexical_cast<string>(signal_info.masterSamplingRate());
  element->LinkEndChild(new TiXmlText(value));
  master_elem->LinkEndChild(element);

  signal_info_elem->LinkEndChild(master_elem);

  TiXmlElement* signals_elem = new TiXmlElement("signals");

  SignalInfo::SignalMap::const_iterator it_signals = signal_info.signals().begin();
  SignalInfo::SignalMap::const_iterator end_signals = signal_info.signals().end();
  for (; it_signals != end_signals; ++it_signals)
  {
    const std::string& type = (*it_signals).first;
    const Signal& signal = (*it_signals).second;

    TiXmlElement* signal_elem = new TiXmlElement("sig");
    signal_elem->SetAttribute("type", type);

    element = new TiXmlElement("blockSize");
    value = lexical_cast<string>(signal.blockSize());
    element->LinkEndChild(new TiXmlText(value));
    signal_elem->LinkEndChild(element);

    element = new TiXmlElement("samplingRate");
    value = lexical_cast<string>(signal.samplingRate());
    element->LinkEndChild(new TiXmlText(value));
    signal_elem->LinkEndChild(element);

    TiXmlElement* channels_elem = new TiXmlElement("channels");
    std::vector<Channel>::const_iterator it_channels = signal.channels().begin();
    std::vector<Channel>::const_iterator end_channels = signal.channels().end();
    for (; it_channels != end_channels; ++it_channels)
    {
      const Channel& channel = (*it_channels);
      element = new TiXmlElement("ch");
      element->SetAttribute("id", channel.id());
      channels_elem->LinkEndChild(element);
    }

    signal_elem->LinkEndChild(channels_elem);

    signals_elem->LinkEndChild(signal_elem);
  }

  signal_info_elem->LinkEndChild(signals_elem);

  config->LinkEndChild(signal_info_elem);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const ReplyMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;

  switch (msg.msgType())
  {
    case ControlMsg::OkReply:
      encodeBaseMsg(msg, "okReply", doc, xml_msg);
      assert(xml_msg != 0);
      xml_msg->LinkEndChild(new TiXmlElement("okReply"));
      break;
    case ControlMsg::ErrorReply:
      encodeBaseMsg(msg, "errorReply", doc, xml_msg);
      assert(xml_msg != 0);
      xml_msg->LinkEndChild(new TiXmlElement("errorReply"));
      break;
    default:
    {
      std::cerr << "Unhandled reply message type '" << msg.msgType() << "'" << endl;
      assert(false);
    }
  }

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeBaseMsg(const ControlMsg& msg, const std::string& xml_msg_type,
    TiXmlDocument& doc, TiXmlElement*& xml_msg)
{
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
  doc.LinkEndChild( decl );

  xml_msg = new TiXmlElement("message");
  doc.LinkEndChild(xml_msg);
  xml_msg->SetAttribute("version", "0.1");

  TiXmlElement* header = new TiXmlElement("header");
  xml_msg->LinkEndChild(header);

  TiXmlElement* type = new TiXmlElement("type");
  type->LinkEndChild(new TiXmlText(xml_msg_type));
  header->LinkEndChild(type);

  TiXmlElement* sender = new TiXmlElement("sender");
  sender->LinkEndChild(new TiXmlText(msg.sender()));
  header->LinkEndChild(sender);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::writeXMLMsg(TiXmlDocument& doc, std::ostream& stream)
{
  stream << doc;
}

//-----------------------------------------------------------------------------

} // namespace tobiss

//-----------------------------------------------------------------------------

// End Of File
