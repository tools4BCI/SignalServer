/**
* @file ssmethods.cpp
*
* @brief A class wtih some methods for the SignalServer
*
**/

// local
#include "signalserver/ssmethods.h"
#include "signalserver/signal_server.h"
#include "config/xml_parser.h"
#include "hardware/hw_access.h"


namespace tobiss
{
using boost::int32_t;
using boost::uint32_t;

using std::vector;

const string CLIENT_XML_CONFIG = "new_client_config.xml";

//-----------------------------------------------------------------------------

SSMethods::SSMethods(SignalServer* server, XMLParser* config, HWAccess* hw_access)
  : server_(server),
  config_(config),
  hw_access_(hw_access)
{
  server_->setSSMethods(this);
}

//-----------------------------------------------------------------------------

SSMethods::~SSMethods()
{
//  if (hw_access_)
//    delete hw_access_;
//  if(server_)
//    delete server_;
//  if(config_)
//    delete config_;
}

//-----------------------------------------------------------------------------

void SSMethods::startServerSettings()
{
  setServerSettings();

  server_->initialize(config_);
  hw_access_->startDataAcquisition();
}

//-----------------------------------------------------------------------------

void SSMethods::setClientConfig(XMLParser* config)
{
  try
  {
    hw_access_->stopDataAcquisition();
    hw_access_ = new HWAccess(server_->getIOService(), *config);
    setServerSettings();
    server_->setServerSettings();
    hw_access_->startDataAcquisition();
    config_ = config;
  }
  catch(ticpp::Exception& e)
  {
    cout << e.what() << endl;
    string ex_str;
    ex_str = "Setting Client-Config caused an error in HWAccess";
    throw(ticpp::Exception(ex_str));
  }
}

//-----------------------------------------------------------------------------

void SSMethods::setServerSettings()
{
  server_->setMasterBlocksize(hw_access_->getMastersBlocksize());
  server_->setMasterSamplingRate(hw_access_->getMastersSamplingRate());
  server_->setAcquiredSignalTypes(hw_access_->getAcquiredSignalTypes());
  server_->setBlockSizesPerSignalType(hw_access_->getBlockSizesPerSignalType());
  server_->setSamplingRatePerSignalType(hw_access_->getSamplingRatePerSignalType());
  server_->setChannelNames(hw_access_->getChannelNames());

  vector<int32_t> v1;
  vector<pair<float, float> > v3;
  vector<std::string> v4;
  vector<uint32_t> v5;
  //  vector<int> v2;
  for(int i = 0; i<17; i++)
  {
    v1.push_back(250);
//    v2.push_back(9);
    v3.push_back(pair<float, float>(0.45, 0.77));
    v4.push_back("temp_description");
    v5.push_back(123);
  }
  server_->setPhysicalRange(v1);
  server_->setDigitalRange(v1);
//  server_->setDataType(v2);
  server_->setBpFilterSettings(v3);
  server_->setNFilterSettings(v3);
  server_->setDescription(v4);
  server_->setDeviceId(v5);

//  server_->setPhysicalRange(hw_access_->getPhysicalRange());
//  server_->setDigitalRange(hw_access_->getDigitalRange());
//  server_->setDataType(hw_access_->getDataType());
//  server_->setBpFilterSettings(hw_access_->getBpFilterSettings());
//  server_->setNFilterSettings(hw_access_->getNFilterSettings());
//  server_->setDescription(hw_access_->getDescription());
//  server_->setDeviceId(hw_access_->getDeviceId());
}

//-----------------------------------------------------------------------------


} // Namespace tobiss

