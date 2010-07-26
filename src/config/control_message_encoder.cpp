/**
* @control_message_encoder.cpp
*
* @brief \TODO.
*
**/

// local
#include "definitions/constants.h"

#include "config/control_message_encoder.h"

namespace tobiss
{

using std::cout;
using std::endl;
using std::string;
using std::stringstream;

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const KeepAliveMsg& msg, std::ostream& stream)
{
  ticpp::Document doc;
  ticpp::Element* xml_msg = 0;
  encodeBaseMsg(msg,"alive", doc, xml_msg);
  assert(xml_msg != 0);

  ticpp::Element* cmd = new ticpp::Element("alive");
  xml_msg->LinkEndChild(cmd);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const GetConfigMsg& msg, std::ostream& stream)
{
  ticpp::Document doc;
  ticpp::Element* xml_msg = 0;
  encodeBaseMsg(msg,"getConfig", doc, xml_msg);
  assert(xml_msg != 0);

  ticpp::Element* cmd = new ticpp::Element("getConfig");
  xml_msg->LinkEndChild(cmd);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const StopTransmissionMsg& msg, std::ostream& stream)
{
  ticpp::Document doc;
  ticpp::Element* xml_msg = 0;
  encodeBaseMsg(msg,"stopTransmission", doc, xml_msg);
  assert(xml_msg != 0);

  ticpp::Element* cmd = new ticpp::Element("stopTransmission");
  xml_msg->LinkEndChild(cmd);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const GetDataConnectionMsg& msg, std::ostream& stream)
{
  ticpp::Document doc;
  ticpp::Element* xml_msg = 0;
  encodeBaseMsg(msg,"getDataConnection", doc, xml_msg);
  assert(xml_msg != 0);

  ticpp::Element* cmd_element = new ticpp::Element("getDataConnection");
  ticpp::Element* element = new ticpp::Element("connectionType");
  element->SetText(msg.connectionType() == GetDataConnectionMsg::Tcp ? "tcp" : "udp");
  cmd_element->LinkEndChild(element);
  xml_msg->LinkEndChild(cmd_element);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const DataConnectionMsg& msg, std::ostream& stream)
{
  ticpp::Document doc;
  ticpp::Element* xml_msg = 0;
  encodeBaseMsg(msg,"dataConnection", doc, xml_msg);
  assert(xml_msg != 0);

  ticpp::Element* reply_element = new ticpp::Element("dataConnection");

  ticpp::Element* element = 0;

  if (!msg.address().empty())
  {
    element = new ticpp::Element("address");
    element->SetText(msg.address());
    reply_element->LinkEndChild(element);
  }

  element = new ticpp::Element("port");
  element->SetText(msg.port());
  reply_element->LinkEndChild(element);

  xml_msg->LinkEndChild(reply_element);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const StartTransmissionMsg& msg, std::ostream& stream)
{
  ticpp::Document doc;
  ticpp::Element* xml_msg = 0;
  encodeBaseMsg(msg,"startTransmission", doc, xml_msg);
  assert(xml_msg != 0);

  ticpp::Element* cmd_element = new ticpp::Element("startTransmission");
  xml_msg->LinkEndChild(cmd_element);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const ConfigMsg& msg, std::ostream& stream)
{
  ticpp::Document doc;
  ticpp::Element* xml_msg = 0;
  encodeBaseMsg(msg,"config", doc, xml_msg);
  assert(xml_msg != 0);

  ticpp::Element* config = new ticpp::Element("config");

  xml_msg->LinkEndChild(config);

  const SubjectInfo& subject = msg.subject_info;

  ticpp::Element* subject_elem = new ticpp::Element("subject");

  ticpp::Element* element = 0;

  element = new ticpp::Element("id");
  element->SetText(subject.id());
  subject_elem->LinkEndChild(element);

  element = new ticpp::Element("firstName");
  element->SetText(subject.firstName());
  subject_elem->LinkEndChild(element);

  element = new ticpp::Element("surname");
  element->SetText(subject.surname());
  subject_elem->LinkEndChild(element);

  element = new ticpp::Element("sex");
  element->SetText(subject.sex() == SubjectInfo::Male ? "m" : "f");
  subject_elem->LinkEndChild(element);

  element = new ticpp::Element("birthday");
  element->SetText(subject.birthday());
  subject_elem->LinkEndChild(element);

  element = new ticpp::Element("handedness");
  element->SetText(subject.handedness() == SubjectInfo::RightHanded ? "r" : "l");
  subject_elem->LinkEndChild(element);

  element = new ticpp::Element("medication");
  element->SetText(subject.medication());
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
        element = new ticpp::Element("glasses");
        element->SetText(value_as_str);
        subject_elem->LinkEndChild(element);
        break;
      case SubjectInfo::Smoking:
        element = new ticpp::Element("smoker");
        element->SetText(value_as_str);
        subject_elem->LinkEndChild(element);
        break;
    }
  }

  config->LinkEndChild(subject_elem);

  const SignalInfo& signal_info = msg.signal_info;

  ticpp::Element* signal_info_elem = new ticpp::Element("signalInfo");

  ticpp::Element* master_elem = new ticpp::Element("master");

  element = new ticpp::Element("blockSize");
  element->SetText(signal_info.masterBlockSize());
  master_elem->LinkEndChild(element);

  element = new ticpp::Element("samplingRate");
  element->SetText(signal_info.masterSamplingRate());
  master_elem->LinkEndChild(element);

  signal_info_elem->LinkEndChild(master_elem);

  ticpp::Element* signals_elem = new ticpp::Element("signals");

  SignalInfo::SignalMap::const_iterator it_signals = signal_info.signals().begin();
  SignalInfo::SignalMap::const_iterator end_signals = signal_info.signals().end();
  for (; it_signals != end_signals; ++it_signals)
  {
    const std::string& type = (*it_signals).first;
    const Signal& signal = (*it_signals).second;

    ticpp::Element* signal_elem = new ticpp::Element("sig");
    signal_elem->SetAttribute("type", type);

    element = new ticpp::Element("blockSize");
    element->SetText(signal.blockSize());
    signal_elem->LinkEndChild(element);

    element = new ticpp::Element("samplingRate");
    element->SetText(signal.samplingRate());
    signal_elem->LinkEndChild(element);

    ticpp::Element* channels_elem = new ticpp::Element("channels");
    element->SetText(signal.samplingRate());

    std::vector<Channel>::const_iterator it_channels = signal.channels().begin();
    std::vector<Channel>::const_iterator end_channels = signal.channels().end();
    for (; it_channels != end_channels; ++it_channels)
    {
      const Channel& channel = (*it_channels);
      element = new ticpp::Element("ch");
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
  ticpp::Document doc;
  ticpp::Element* xml_msg = 0;

  switch (msg.msgType())
  {
    case ControlMsg::OkReply:
      encodeBaseMsg(msg, "okReply", doc, xml_msg);
      assert(xml_msg != 0);
      xml_msg->LinkEndChild(new ticpp::Element("okReply"));
      break;
    case ControlMsg::ErrorReply:
      encodeBaseMsg(msg, "errorReply", doc, xml_msg);
      assert(xml_msg != 0);
      xml_msg->LinkEndChild(new ticpp::Element("errorReply"));
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
    ticpp::Document& doc, ticpp::Element*& xml_msg)
{
  ticpp::Declaration* decl = new ticpp::Declaration( "1.0", "", "" );
  doc.LinkEndChild(decl);

  xml_msg = new ticpp::Element("message");
  doc.LinkEndChild(xml_msg);

  xml_msg->SetAttribute("version", "0.1");

  ticpp::Element* header = new ticpp::Element("header");
  xml_msg->LinkEndChild(header);

  ticpp::Element* type = new ticpp::Element("type");
  type->SetText(xml_msg_type);
  header->LinkEndChild(type);

  ticpp::Element* sender = new ticpp::Element("sender");
  sender->SetText(msg.sender());
  header->LinkEndChild(sender);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::writeXMLMsg(ticpp::Document& doc, std::ostream& stream)
{
  stream << doc;

  {
    cout << ">>> XML Message:" << endl;
    stringstream string_str;
    string_str << doc;
    cout << string_str.str() << endl;
    cout << "<<< XML Message" << endl;
  }
}

//-----------------------------------------------------------------------------

} // namespace tobiss

//-----------------------------------------------------------------------------

// End Of File
