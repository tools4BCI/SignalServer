#ifndef TIA_EXCEPTIONS_H
#define TIA_EXCEPTIONS_H

#include <stdexcept>

namespace tia
{

class TiAException : public std::runtime_error
{
public:
    TiAException (std::string const& what) : std::runtime_error (what) {}
    virtual ~TiAException () throw () {}
};

class TiALostConnection : public TiAException
{
public:
    TiALostConnection (std::string const& what) : TiAException (what) {}
    virtual ~TiALostConnection () throw () {}
};

}

#endif // TIA_EXCEPTIONS_H
