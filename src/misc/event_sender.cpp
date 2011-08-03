/*
    This file is part of the TOBI SignalServer.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU General Public License Usage
    Alternatively, this file may be used under the terms of the GNU
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file gpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/gpl.html.

    In case of GNU General Public License Usage ,the TOBI SignalServer
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the TOBI SignalServer. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: SignalServer@tobi-project.org
*/

#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using namespace std;
using namespace tobiss;

int main()
{
  boost::asio::io_service io_service;
  boost::system::error_code ec;
  boost::asio::ip::address addr;
  string str;

  addr = addr.from_string("127.0.0.1");

  udp::socket socket(io_service);
  socket.open(boost::asio::ip::udp::v4(), ec);

  if (!ec)
  {

    while(cin >> str)
      {
        if(str == "q")
          break;

  //      string temp = "1234\n456\n";
        socket.send_to(boost::asio::buffer(str + '\n'),
                        udp::endpoint( addr, 12344));
	sleep(1);
      }

  }

  return(0);
}
