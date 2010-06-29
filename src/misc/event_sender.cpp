#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using namespace std;

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
