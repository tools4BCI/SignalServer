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
*
* @brief
*
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
