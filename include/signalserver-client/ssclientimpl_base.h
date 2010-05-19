/**
* @file server_server.h
*
* @brief
*
**/

#ifndef SSCLIENTIMPL_BASE_H
#define SSCLIENTIMPL_BASE_H

// local
#include "config/ss_meta_info.h"
#include "datapacket/data_packet.h"

//-----------------------------------------------------------------------------

class SSConfig
{
  public:
    tobiss::SubjectInfo subject_info;
    tobiss::SignalInfo  signal_info;
};

//-----------------------------------------------------------------------------

class SSClientImplBase
{
  public:
    SSClientImplBase(){}

    virtual ~SSClientImplBase(){}

    virtual void connect(const std::string& address, boost::uint16_t port) = 0;

    virtual bool connected() const = 0;

    virtual void disconnect() = 0;

    virtual void requestConfig() = 0;

    virtual SSConfig config() const = 0;

    virtual void startReceiving(bool use_udp_bc) = 0;

    virtual bool receiving() const = 0;

    virtual void stopReceiving() = 0;

    virtual void getDataPacket(DataPacket& packet) = 0;
};

//-----------------------------------------------------------------------------

#endif // SSCLIENTIMPL_BASE_H
