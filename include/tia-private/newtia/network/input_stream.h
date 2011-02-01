#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include <string>

namespace tia
{

//-----------------------------------------------------------------------------
class InputStream
{
public:
    virtual ~InputStream () {}
    virtual std::string readLine (unsigned max_length) = 0;
    virtual std::string readString (unsigned length) = 0;
    virtual char readCharacter () = 0;
};

}

#endif // INPUT_STREAM_H
