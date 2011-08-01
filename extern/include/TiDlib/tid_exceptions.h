#ifndef TID_EXCEPTIONS_H
#define TID_EXCEPTIONS_H

#include <stdexcept>

namespace TiD
{

class TiDException : public std::runtime_error
{
public:
    TiDException (std::string const& what) : std::runtime_error (what) {}
    virtual ~TiDException () throw () {}
};

class TiDLostConnection : public TiDException
{
public:
    TiDLostConnection (std::string const& what) : TiDException (what) {}
    virtual ~TiDLostConnection () throw () {}
};

}

#endif // TID_EXCEPTIONS_H
