#include "tia-private/server/boost_socket_impl.h"

using std::string;

namespace tia
{

//-----------------------------------------------------------------------------
string BoostTCPSocketImpl::readLine (unsigned max_length)
{
    return "";
}


//-----------------------------------------------------------------------------
string BoostTCPSocketImpl::readString (unsigned max_length)
{
    return "";
}

//-----------------------------------------------------------------------------
char BoostTCPSocketImpl::readCharacter ()
{
    return 0;
}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::waitForData ()
{

}

//-----------------------------------------------------------------------------
void BoostTCPSocketImpl::sendString (string const& str)
{

}

}
