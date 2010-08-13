/**
* @control_message_decoder.cpp
*
* @brief \TODO.
*
**/

// Boost
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>

// TICPP
#include "ticpp/ticpp.h"

// local
#include "definitions/constants.h"

#include "config/control_message_decoder.h"
#include "config/ss_meta_info.h"

namespace tobiss
{

using boost::lexical_cast;
using boost::uint16_t;
using boost::uint32_t;

using std::cout;
using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------

ControlMsgDecoderXML::ControlMsgDecoderXML() :
  tixml_doc_(0), xml_doc_(0), xml_msg_header_(0)
{
  tixml_doc_ = new TiXmlDocument();
  xml_doc_ = new ticpp::Document(tixml_doc_);

  prototypes_["alive"]                    = ControlMsgHandle(new KeepAliveMsg());
  prototypes_["getConfig"]                = ControlMsgHandle(new GetConfigMsg());
  prototypes_["startTransmission"]        = ControlMsgHandle(new StartTransmissionMsg());
  prototypes_["getDataConnection"]        = ControlMsgHandle(new GetDataConnectionMsg());
  prototypes_["dataConnection"]           = ControlMsgHandle(new DataConnectionMsg());
  prototypes_["stopTransmission"]         = ControlMsgHandle(new StopTransmissionMsg());
  prototypes_["config"]                   = ControlMsgHandle(new ConfigMsg());
  prototypes_["sendConfig"]               = ControlMsgHandle(new SendConfigMsg());
  prototypes_["okReply"]                  = ControlMsgHandle(ReplyMsg::ok().clone());
  prototypes_["errorReply"]               = ControlMsgHandle(ReplyMsg::error().clone());
}

//-----------------------------------------------------------------------------

ControlMsgDecoderXML::~ControlMsgDecoderXML()
{
  if(xml_doc_)
    delete tixml_doc_;

  if(xml_doc_)
    delete xml_doc_;
}

//-----------------------------------------------------------------------------

ControlMsg* ControlMsgDecoderXML::decodeMsg()
{
  return decodeMsgInternal();
}

//-----------------------------------------------------------------------------

ControlMsg* ControlMsgDecoderXML::decodeMsgInternal()
{
  if (istream_ == 0 || !istream_->good()) return 0;

  xml_doc_->Clear();
  xml_msg_header_ = 0;

  *istream_ >> *xml_doc_;

  if (tixml_doc_->Error())
  {
    // TODO: do a proper error handling
    cerr << xml_doc_->BuildDetailedErrorString() << endl;
    return 0;
  }

  cout << ">>> XML message " << endl;
  tixml_doc_->Print();
  cout << "<<< XML message " << endl;

  ticpp::Element* message = xml_doc_->FirstChildElement("message", false);
  if (message == 0)
  {
    // TODO: do a proper error handling
    cerr << "Missing tag 'message'" << endl;
    return 0;
  }

  xml_msg_header_ = message->FirstChildElement("header", false);
  if (xml_msg_header_ == 0)
  {
    // TODO: do a proper error handling
    cerr << "Missing tag 'header'" << endl;
    return 0;
  }

  std::string xml_msg_type;
  if (!getXmlMsgType(*xml_msg_header_, xml_msg_type))
  {
     // TODO: do a proper error handling
     return 0;
  }

  std::map<std::string, ControlMsgHandle>::iterator itPrototypes =
    prototypes_.find(xml_msg_type);

  if (itPrototypes == prototypes_.end())
  {
    // TODO: do a proper error handling
    cerr << "Unknown control message type '" << xml_msg_type << "'" << endl;
    return 0;
  }

  ControlMsg* msg = (*itPrototypes).second->clone();
  assert(msg != 0);
  msg->readMsg(*this);

  return msg;
}

//-----------------------------------------------------------------------------

bool ControlMsgDecoderXML::getXmlMsgType(ticpp::Element& header, std::string& type) const
{
  ticpp::Element* element = header.FirstChildElement("type", false);
  if (element == 0)
  {
    // TODO: do a proper error handling
    cerr << "Missing tag 'type' in control message header" << endl;
    return false;
  }

  type = element->GetText();
  return true;
}

//-----------------------------------------------------------------------------

bool ControlMsgDecoderXML::decodeHeader(ControlMsg& msg)
{
    ticpp::Element* element = xml_msg_header_->FirstChildElement("sender", false);
    if (element == 0 || element->Value() != "sender")
    {
      // TODO: do a proper error handling
      cerr << "Missing tag 'sender' in control message header" << endl;
      return false;
    }

    std::string text = element->GetTextOrDefault("");
    msg.setSender(text);

    return true;
}

//-----------------------------------------------------------------------------

void ControlMsgDecoderXML::decodeMsg(KeepAliveMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg KeepAliveMsg" << endl;
  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding KeepAliveMsg" << endl;
    return;
  }
  // TODO: parse message body
}

//-----------------------------------------------------------------------------

void ControlMsgDecoderXML::decodeMsg(GetConfigMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg GetConfigMsg" << endl;
  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding GetConfigMsg" << endl;
    return;
  }
  // TODO: parse message body
}

//-----------------------------------------------------------------------------

void ControlMsgDecoderXML::decodeMsg(StopTransmissionMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg StopTransmissionMsg" << endl;

  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding StopTransmissionMsg" << endl;
    return;
  }
  // TODO: parse message body
}

//-----------------------------------------------------------------------------

void ControlMsgDecoderXML::decodeMsg(StartTransmissionMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg StartTransmissionMsg" << endl;

  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding StartTransmissionMsg" << endl;
    return;
  }

  ticpp::Element* start_trans_element = xml_msg_header_->NextSiblingElement();
  if (start_trans_element == 0 || start_trans_element->Value() != "startTransmission")
  {
    // TODO: do a proper error handling
    cerr << "Missing element 'startTransmission' in StartTransmissionMsg" << endl;
    return;
  }
}

//-----------------------------------------------------------------------------

void ControlMsgDecoderXML::decodeMsg(GetDataConnectionMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg GetDataConnectionMsg" << endl;

  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding GetDataConnectionMsg" << endl;
    return;
  }

  ticpp::Element* get_data_conn_element = xml_msg_header_->NextSiblingElement();
  if (get_data_conn_element == 0 || get_data_conn_element->Value() != "getDataConnection")
  {
    // TODO: do a proper error handling
    cerr << "Missing element 'getDataConnection' in GetDataConnectionMsg" << endl;
    return;
  }

  std::string element_name = "connectionType";
  ticpp::Element* element = get_data_conn_element->FirstChildElement(element_name, false);
  if (element != 0)
  {
    std::string value = element->GetTextOrDefault("");
    if (value == "tcp")
      msg.setConnectionType(GetDataConnectionMsg::Tcp);
    else if (value == "udp")
      msg.setConnectionType(GetDataConnectionMsg::Udp);
    else
      cerr << "Invalid value for element '" << element_name << endl;
  }
  else
    cerr << "Missing element '" << element_name << " in element 'getDataConnection'" << endl;

}

//-----------------------------------------------------------------------------

void ControlMsgDecoderXML::decodeMsg(DataConnectionMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg DataConnectionMsg" << endl;

  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding GetDataConnectionMsg" << endl;
    return;
  }

  ticpp::Element* data_conn_element = xml_msg_header_->NextSiblingElement();
  if (data_conn_element == 0 || data_conn_element->Value() != "dataConnection")
  {
    // TODO: do a proper error handling
    cerr << "Missing element 'dataConnection' in DataConnectionMsg" << endl;
    return;
  }

  std::string element_name = "address";
  ticpp::Element* element = data_conn_element->FirstChildElement(element_name, false);
  if (element != 0)
  {
    std::string address = element->GetTextOrDefault("");
    msg.setAddress(address);
  }

  element_name = "port";
  element = data_conn_element->FirstChildElement(element_name, false);
  if (element != 0)
  {
    try {
      uint16_t value = 0;
      element->GetText(&value);
      msg.setPort(value);
    }
    catch(...)
    {
      cerr << "Invalid value for element '" << element_name << endl;
    }
  }
  else
    cerr << "Missing element '" << element_name << " in element 'dataConnection'" << endl;
}

//-----------------------------------------------------------------------------

void ControlMsgDecoderXML::decodeMsg(ConfigMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg ConfigMsg"  << endl;

  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding ConfigMsg" << endl;
    return;
  }

  ticpp::Element* config_element = xml_msg_header_->NextSiblingElement();
  if (config_element == 0 || config_element->Value() != "config")
  {
    // TODO: do a proper error handling
    cerr << "Missing element 'config' in ConfigMsg" << endl;
    return;
  }

  ticpp::Element* subject_element = config_element->FirstChildElement("subject", false);
  if (subject_element == 0)
  {
    // TODO: do a proper error handling
    cerr << "Missing element 'subject' in ConfigMsg" << endl;
    return;
  }

  std::string element_name;
  ticpp::Element* element = 0;

  element_name = "id";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
    msg.subject_info.setId(element->GetText());
  else
    cerr << "Missing element '" << element_name << "' in element 'subject'" << endl;

  element_name = "firstName";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
    msg.subject_info.setFirstName(element->GetText());
  else
    cerr << "Missing element '" << element_name << "' in element 'subject'" << endl;

  element_name = "surname";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
    msg.subject_info.setSurname(element->GetText());
  else
    cerr << "Missing element '" << element_name << "' in element 'subject'" << endl;

  element_name = "birthday";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
    msg.subject_info.setBirthday(element->GetText());
  else
    cerr << "Missing element '" << element_name << "' in element 'subject'" << endl;

  element_name = "medication";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
    msg.subject_info.setBirthday(element->GetText());
  else
    cerr << "Missing element '" << element_name << "' in element 'subject'" << endl;

  element_name = "sex";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
  {
    if (element->GetText() == "m")
      msg.subject_info.setSex(SubjectInfo::Male);
    else if (element->GetText() == "f")
      msg.subject_info.setSex(SubjectInfo::Female);
    else
      cerr << "Invalid value for element '" << element_name << "' in element 'subject'" << endl;
  }
  else
    cerr << "Missing element '" << element_name << "' in subject" << endl;

  element_name = "handedness";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
  {
    if (element->GetText() == "r")
      msg.subject_info.setHandedness(SubjectInfo::RightHanded);
    else if (element->GetText() == "l")
      msg.subject_info.setHandedness(SubjectInfo::LeftHanded);
    else
      cerr << "Invalid value for element '" << element_name << "' in element 'subject'" << endl;
  }
  else
    cerr << "Missing element '" << element_name << "' in subject" << endl;

  // Optional elements

  element_name = "glasses";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
  {
    if (element->GetText() == "yes")
      msg.subject_info.setShortInfo(SubjectInfo::Glasses, SubjectInfo::Yes);
    else if (element->GetText() == "no")
      msg.subject_info.setShortInfo(SubjectInfo::Glasses, SubjectInfo::No);
    else
      cerr << "Invalid value for element '" << element_name << "' in element 'subject'" << endl;
  }

  element_name = "smoker";
  element = subject_element->FirstChildElement(element_name, false);
  if (element != 0)
  {
    if (element->GetText() == "yes")
      msg.subject_info.setShortInfo(SubjectInfo::Smoking, SubjectInfo::Yes);
    else if (element->GetText() == "no")
      msg.subject_info.setShortInfo(SubjectInfo::Smoking, SubjectInfo::No);
    else
      cerr << "Invalid value for element '" << element_name << "' in element 'subject'" << endl;
  }

  ticpp::Element* signal_info_element = config_element->FirstChildElement("signalInfo", false);
  if (signal_info_element == 0)
  {
    // TODO: do a proper error handling
    cerr << "Missing element 'signalInfo' in ConfigMsg" << endl;
    return;
  }

  ticpp::Element* master_element = signal_info_element->FirstChildElement("master", false);
  if (master_element != 0)
  {
    element_name = "samplingRate";
    element = master_element->FirstChildElement(element_name, false);
    if (element != 0)
    {
      try {
        uint16_t value = 0;
        element->GetText(&value);
        msg.signal_info.setMasterSamplingRate(value);
      }
      catch(...)
      {
        cerr << "Invalid value for '" << element_name << "' in element 'master'" << endl;
      }
    }
    else
      cerr << "Missing element '" << element_name << "' in element 'master'" << endl;

    element_name = "blockSize";
    element = master_element->FirstChildElement(element_name, false);
    if (element != 0)
    {
      try {
        uint16_t value = 0;
        element->GetText(&value);
        msg.signal_info.setMasterBlockSize(value);
      }
      catch(...)
      {
        cerr << "Invalid value for '" << element_name << "' in element 'master'" << endl;
      }
    }
    else
      cerr << "Missing element '" << element_name << "' in element 'master'" << endl;
  }
  else
  {
    cerr << "Missing element 'master' in element 'signalInfo'" << endl;
    return;
  }

  ticpp::Element* signals_element = signal_info_element->FirstChildElement("signals", false);

  if (signals_element != 0)
  {
    ticpp::Element* sig_element = signals_element->FirstChildElement(false);
    for (; sig_element != 0; sig_element = sig_element->NextSiblingElement(false))
    {
      if (sig_element->Value() != "sig")
      {
        cerr << "Misplaced element '" << sig_element->Value() << "' in element 'signals'" << endl;
        continue;
      }

      std::string signal_type = sig_element->GetAttributeOrDefault("type", "");
      if (signal_type.empty())
      {
        cerr << "Element 'sig' without attribute 'type' in element signals" << endl;
        continue;
      }

      Signal signal;
      signal.setType(signal_type);

      element_name = "blockSize";
      element = sig_element->FirstChildElement(element_name, false);
      if (element != 0)
      {
        try {
          uint16_t value = 0;
          element->GetText(&value);
          signal.setBlockSize(value);
        }
        catch(...)
        {
          cerr << "Invalid value for '" << element_name << "' in element 'sig'" << endl;
        }
      }
      else
        cerr << "Missing element '" << element_name << "' in element 'sig'" << endl;

      element_name = "samplingRate";
      element = sig_element->FirstChildElement(element_name, false);
      if (element != 0)
      {
        try {
          uint16_t value = 0;
          element->GetText(&value);
          signal.setSamplingRate(value);
        }
        catch(...)
        {
          cerr << "Invalid value for '" << element_name << "' in element 'sig'" << endl;
        }
      }
      else
        cerr << "Missing element '" << element_name << "' in element 'sig'" << endl;

      ticpp::Element* channels_element = sig_element->FirstChildElement("channels", false);

      if (channels_element != 0)
      {
        ticpp::Element* ch_element = channels_element->FirstChildElement(false);
        for (; ch_element != 0; ch_element = ch_element->NextSiblingElement(false))
        {
          if (ch_element->Value() != "ch")
          {
            cerr << "Misplaced element '" << ch_element->Value() << "' in element 'signals'" << endl;
            continue;
          }

          Channel ch;
          ch.setId(ch_element->GetAttributeOrDefault("id", ""));
          signal.channels().push_back(ch);
        }
      }
      else
        cerr << "Missing element 'channels' in element 'sig'" << endl;

      msg.signal_info.signals().insert(make_pair(signal_type, signal));
    }
  }
  else
    cerr << "Missing element 'signals' in 'signalInfo'" << endl;
}

//-----------------------------------------------------------------------------

void ControlMsgDecoderXML::decodeMsg(ReplyMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg ReplyMsg"  << endl;

  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding ReplyMsg" << endl;
    return;
  }

  switch (msg.msgType())
  {
    case ControlMsg::OkReply:
    {
      ticpp::Element* reply_element = xml_msg_header_->NextSiblingElement();
      if (reply_element == 0 || reply_element->Value() != "okReply")
      {
        // TODO: do a proper error handling
        cerr << "Missing element 'okReply' in ReplyMsg" << endl;
        return;
      }
      break;
    }
    case ControlMsg::ErrorReply:
    {
      ticpp::Element* reply_element = xml_msg_header_->NextSiblingElement();
      if (reply_element == 0 || reply_element->Value() != "errorReply")
      {
        // TODO: do a proper error handling
        cerr << "Missing element 'errorReply' in ReplyMsg" << endl;
        return;
      }
      break;
    }

    default:
    {
      // TODO: do a proper error handling
      cerr << "Catastrophic error - aborting" << endl;
      assert(false);
    }
  }
}

//-----------------------------------------------------------------------------
void ControlMsgDecoderXML::decodeMsg(SendConfigMsg& msg)
{
  cout << "Calling ControlMsgDecoderXML::decodeMsg SendConfigMsg" << endl;
  if (!decodeHeader(msg))
  {
    // TODO: do a proper error handling
    cerr << "Error decoding SendConfigMsg" << endl;
    return;
  }

  ticpp::Element* config_element = xml_msg_header_->NextSiblingElement();
  if (config_element == 0 || config_element->Value() != "sendConfig")
  {
    // TODO: do a proper error handling
    cerr << "Missing element 'sendConfig' in SendConfigMsg" << endl;
    return;
  }
  else
  {
    std::string text = config_element->GetText();
    msg.setConfigString(text);
  }

}

//-----------------------------------------------------------------------------

} // Namespace tobiss

// End Of File
