#ifndef DATA_SERVER_H
#define DATA_SERVER_H

namespace tia
{

typedef unsigned ConnectionID;
typedef unsigned short Port;

//-----------------------------------------------------------------------------
/// DataServer
///
/// @brief base class for data servers which broadcast datapackets
class DataServer
{
public:
    virtual ~DataServer () {}

    virtual Port localPort (ConnectionID connection) const = 0;

    virtual ConnectionID addConnection (bool udp) = 0;

    virtual bool hasConnection (ConnectionID connection) const = 0;

    virtual bool transmitting (ConnectionID connection) const = 0;

    virtual bool removeConnection (ConnectionID connection) = 0;

    virtual void startTransmission (ConnectionID connection) = 0;

    virtual void stopTransmission (ConnectionID connection) = 0;
};


}

#endif // DATA_SERVER_H
