/**
* @file ssconfig.h
*
* @brief
*
**/

#ifndef SSCONFIG_BASE_H
#define SSCONFIG_BASE_H

// local
#include "config/ss_meta_info.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class SSConfig
*
* @brief The core of the Signal Server
*
* @todo
*/
class SSConfig
{
  public:
  /**
  * @brief
  */
  SubjectInfo subject_info;
  /**
  * @brief
  */
  SignalInfo  signal_info;
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // SSCONFIG_BASE_H
