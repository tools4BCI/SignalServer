/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

/**
* @file tia_client_impl_base.h
*
* @brief Declaration of the abstract base class of the TiAClient implementation
*
**/

#ifndef TIA_CLIENT_IMPL_BASE_H
#define TIA_CLIENT_IMPL_BASE_H

#include <string>

namespace tia
{
// forward declarations;
class SSConfig;
class DataPacket;

//-----------------------------------------------------------------------------
/**
* @class TiAClientImplBase
*
* @brief Abstract Base of the TiAClient Implementation
*/
class TiAClientImplBase
{
public:
  /**
   * @brief Constructor
   */
  TiAClientImplBase(){}
  /**
   * @brief Destructor
   */
  virtual ~TiAClientImplBase(){}
  /**
   * @brief Establish a connection to a TOBI SignalServer
   * \sa connected(), disconnect()
   */
  virtual void connect(const std::string& address, short unsigned port) = 0;
  /**
   * @brief Tells if the client is connected to the server,
   *        i.e. if the control connection has been established.
   * \sa connect(), disconnect()
   */
  virtual bool connected() const = 0;
  /**
   * @brief Disconnect from the server, i.e. stops receiving and closes the control connection.
   * \sa connect(), disconnect()
   */
  virtual void disconnect() = 0;
  /**
   * @brief Request the meta data information from the server
   * \sa    config()
   */
  virtual void requestConfig() = 0;
  /**
   * @brief Returns the meta data information requested from the server
   * \sa requestConfig()
   */
  virtual SSConfig config() const = 0;
  /**
   * @brief Turns the client into receiving state
   * \sa stopReceiving(), receiving()
   */
  virtual void startReceiving(bool use_udp_bc) = 0;
  /**
   * @brief Tell if the client if is in receiving state
   * \sa startReceiving(), stopReceiving()
   */
  virtual bool receiving() const = 0;
  /**
   * @brief Stops receiving, i.e. calling getDataPacket() will fail afterwards.
   * \sa receiving(), startReceiving()
   */
  virtual void stopReceiving() = 0;
  /**
   * @brief Gets a packet from the server.
   */
  virtual void getDataPacket(DataPacket& packet) = 0;
  /**
   * @brief Sets the client's data input buffer size to the given value
   */
  virtual void setBufferSize(size_t size) = 0;

  /**
  * @brief todo
  */
  virtual DataPacket* getEmptyDataPacket() = 0;
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // TIA_CLIENT_IMPL_BASE_H
