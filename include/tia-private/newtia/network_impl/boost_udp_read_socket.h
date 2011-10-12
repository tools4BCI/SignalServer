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

#ifndef BOOST_UDP_READ_SOCKET_H
#define BOOST_UDP_READ_SOCKET_H

#include "tia-private/newtia/network/read_socket.h"

#include <boost/asio/ip/udp.hpp>

namespace tia
{

class BoostUDPReadSocket : public ReadSocket
{
public:
    BoostUDPReadSocket (boost::asio::io_service& io_service, boost::asio::ip::udp::endpoint const& endpoint, unsigned buffer_size);
    virtual void setReceiveBufferSize (unsigned size);


    virtual std::string readUntil (char delimiter);
    virtual std::string readUntil (std::string delimiter);
    virtual std::string readString (unsigned length);
    virtual char readCharacter ();
    virtual void waitForData ();

    virtual size_t readBytes (char* data, size_t bytes_to_read) { return 0;}
    virtual size_t getAvailableData (char* data, size_t max_size) { return 0; }

private:
    void readBytes (unsigned num_bytes);
    std::auto_ptr<boost::asio::ip::udp::socket> socket_;
    std::string buffered_string_;
    unsigned buffer_size_;
    std::vector<char> rec_buffer_;
};

}

#endif // BOOST_UDP_READ_SOCKET_H
