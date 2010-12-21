/*
    This file is part of the TOBI signal server.

    The TOBI signal server is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The TOBI signal server is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file ssconfig.h
* @brief This file includes a class storing the TiA config for the subject and the transmitted signals.
**/

#ifndef SSCONFIG_BASE_H
#define SSCONFIG_BASE_H

// local
#include "tia/ss_meta_info.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class SSConfig
*
* @brief This class holds information regarding the subject and the transmitted signals.
*
* @todo Rename class to TiAConfig
*/
class SSConfig
{
  public:
  /**
  * @brief Element holding subject specific information.
  */
  SubjectInfo subject_info;
  /**
  * @brief Element holding information of transmitted signals.
  */
  SignalInfo  signal_info;
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // SSCONFIG_BASE_H
