/**
* @control_message_encoder.cpp
*
* @brief \TODO.
*
**/

// Boost
#include <boost/lexical_cast.hpp>

// local
#include "definitions/constants.h"

#include "config/control_message_encoder.h"

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

      element->SetAttribute("deviceId", lexical_cast<string>(channel.deviceId()));
      element->SetAttribute("description", channel.description());
      element->SetAttribute("physicalRange", lexical_cast<string>(channel.physicalRange()));
      element->SetAttribute("digitalRange", lexical_cast<string>(channel.digitalRange()));

      std::string dataType;
      switch(channel.dataType())
      {
        case Constants::uint8_  : dataType = "uint8"; break;
        case Constants::int8_   : dataType = "int8"; break;
        case Constants::uint16_ : dataType = "uint16"; break;
        case Constants::int16_  : dataType = "int16"; break;
        case Constants::uint32_ : dataType = "uint32"; break;
        case Constants::int32_  : dataType = "int32"; break;
        case Constants::uint64_ : dataType = "uint64"; break;
        case Constants::int64_  : dataType = "int64"; break;
        case Constants::float_  : dataType = "float"; break;
        case Constants::double_ : dataType = "double"; break;
      }
      element->SetAttribute("dataType", dataType);

      std::string filter;
      std::ostringstream temp_filter;
      temp_filter << channel.bpFilter().first;
      filter = temp_filter.str();
      element->SetAttribute("bp_filter_low", filter);
      temp_filter.str("");
      temp_filter.clear();
      temp_filter << channel.bpFilter().second;
      filter = temp_filter.str();
      element->SetAttribute("bp_filter_high", filter);
      temp_filter.str("");
      temp_filter.clear();
      temp_filter << channel.nFilter().first;
      filter = temp_filter.str();
      element->SetAttribute("n_filter_low", filter);
      temp_filter.str("");
      temp_filter.clear();
      temp_filter << channel.nFilter().second;
      filter = temp_filter.str();
      element->SetAttribute("n_filter_high", filter);

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
    case ControlMsg::AliveReply:
      encodeBaseMsg(msg, "aliveReply", doc, xml_msg);
      assert(xml_msg != 0);
      xml_msg->LinkEndChild(new TiXmlElement("aliveReply"));
      break;
    case ControlMsg::ConfigErrorReply:
      encodeBaseMsg(msg, "configErrorReply", doc, xml_msg);
      assert(xml_msg != 0);
      xml_msg->LinkEndChild(new TiXmlElement("configErrorReply"));
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

void ControlMsgEncoderXML::encodeMsg(const SendConfigMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlDocument doc2(msg.configString());
  doc2.LoadFile();
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg, "sendConfig", doc, xml_msg);
  assert(xml_msg != 0);



  TiXmlElement* config = new TiXmlElement("sendConfig");
  config->LinkEndChild(new TiXmlText(lexical_cast<string>(doc2)));
  xml_msg->LinkEndChild(config);

  writeXMLMsg(doc, stream);
}

//-----------------------------------------------------------------------------

void ControlMsgEncoderXML::encodeMsg(const HWConfigMsg& msg, std::ostream& stream)
{
  TiXmlDocument doc;
  TiXmlElement* xml_msg = 0;
  encodeBaseMsg(msg,"hwconfig", doc, xml_msg);
  assert(xml_msg != 0);

  TiXmlElement* hw_config = new TiXmlElement("hwconfig");

  xml_msg->LinkEndChild(hw_config);

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

  hw_config->LinkEndChild(subject_elem);

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
      element->SetAttribute("deviceId", lexical_cast<string>(channel.deviceId()));
      element->SetAttribute("description", channel.description());
      element->SetAttribute("physicalRange", lexical_cast<string>(channel.physicalRange()));
      element->SetAttribute("digitalRange", lexical_cast<string>(channel.digitalRange()));

      std::string dataType;
      switch(channel.dataType())
      {
        case 0: dataType = "uint8"; break;
        case 1: dataType = "int8"; break;
        case 2: dataType = "uint16"; break;
        case 3: dataType = "int16"; break;
        case 4: dataType = "uint32"; break;
        case 5: dataType = "int32"; break;
        case 6: dataType = "uint64"; break;
        case 7: dataType = "int64"; break;
        case 8: dataType = "float"; break;
        case 9: dataType = "double"; break;
      }
      element->SetAttribute("dataType", dataType);

      element->SetAttribute("bp_filter_low", lexical_cast<string>(channel.bpFilter().first));
      element->SetAttribute("bp_filter_high", lexical_cast<string>(channel.bpFilter().second));
      element->SetAttribute("n_filter_low", lexical_cast<string>(channel.nFilter().first));
      element->SetAttribute("n_filter_high", lexical_cast<string>(channel.nFilter().second));

      channels_elem->LinkEndChild(element);
    }

    signal_elem->LinkEndChild(channels_elem);

    signals_elem->LinkEndChild(signal_elem);
  }

  signal_info_elem->LinkEndChild(signals_elem);

  hw_config->LinkEndChild(signal_info_elem);

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
