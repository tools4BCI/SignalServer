/**
* @file ssmethods.h
*
* @brief
*
**/

#ifndef SSMETHODS_H
#define SSMETHODS_H

// STL
#include <map>

#ifdef WIN32
  #include  <conio.h>
  #include  <stdio.h>
#endif

// Boost
#include <boost/asio.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility.hpp>

// local
#include "definitions/constants.h"

namespace tobiss
{
// forward declarations
class SignalServer;
class XMLParser;
class HWAccess;

//-----------------------------------------------------------------------------

/**
* @class SSMethods
*
* @brief The core of the Signal Server
*
*
*
* @todo
*/
class SSMethods
{
    // Methods
    //-----------------------------------
  public:
    /**
    * @brief Constructor
    */
    SSMethods(SignalServer* server, XMLParser* config, HWAccess* hw_access);

    /**
     * @brief Destructor
     */
    virtual ~SSMethods();

    /**
    * @brief Starts HWAccess and passes config-information to server
    */
    void startServerSettings();

    /**
    * @brief Starts HWAccess and passes config-information to server
    * but with the new config sent by the client
    */
    void setClientConfig(XMLParser* config);

    /**
    * @brief Returns reference to HWAccess
    */
    HWAccess* getHWAccess() {return hw_access_;}

  private:
    SignalServer*   server_;
    XMLParser*      config_;
    HWAccess*       hw_access_;

};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // SSMETHODS_H
