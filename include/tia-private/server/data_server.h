#ifndef DATA_SERVER_H
#define DATA_SERVER_H

namespace tia
{

typedef unsigned ConnectionID;
typedef unsigned short Port;

class DataServer
{
public:
    virtual ~DataServer () {}

    virtual Port localPort (ConnectionID connection) const = 0;

    virtual ConnectionID addConnection () = 0;

    virtual bool removeConnection (ConnectionID connection) = 0;

    virtual void startTransmission (ConnectionID connection) = 0;

    virtual void stopTransmission (ConnectionID connection) = 0;
};


}

#endif // DATA_SERVER_H
