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
  config_ = config;

  hw_access_->stopDataAcquisition();
  hw_access_ = new HWAccess(server_->getIOService(), *config_);

  setServerSettings();

  server_->setServerSettings();
  hw_access_->startDataAcquisition();
}

//-----------------------------------------------------------------------------
//void setDataType(const std::vector<DataType>& data_type)
//  { data_type_ = data_type; }

void SSMethods::setServerSettings()
{
  server_->setMasterBlocksize(hw_access_->getMastersBlocksize());
  server_->setMasterSamplingRate(hw_access_->getMastersSamplingRate());
  server_->setAcquiredSignalTypes(hw_access_->getAcquiredSignalTypes());
  server_->setBlockSizesPerSignalType(hw_access_->getBlockSizesPerSignalType());
  server_->setSamplingRatePerSignalType(hw_access_->getSamplingRatePerSignalType());
  server_->setChannelNames(hw_access_->getChannelNames());
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

