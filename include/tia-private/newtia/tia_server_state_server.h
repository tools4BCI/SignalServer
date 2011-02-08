#ifndef TIA_SERVER_STATE_SERVER_H
#define TIA_SERVER_STATE_SERVER_H


namespace tia
{

//-----------------------------------------------------------------------------
class TiAServerStateServer
{
public:
    virtual ~TiAServerStateServer () {}
    unsigned getPort () const;
};

}

#endif // TIA_SERVER_STATE_SERVER_H
