#ifndef READ_SOCKET_H
#define READ_SOCKET_H

#include "input_stream.h"

namespace tia
{

//-----------------------------------------------------------------------------
class ReadSocket : public InputStream
{
public:
    virtual ~ReadSocket () {}
    virtual void waitForData () = 0;
};

}

#endif // READ_SOCKET_H
