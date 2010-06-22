
#include <iostream>
#include <fstream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "hardware/data_packet.h"

using boost::asio::ip::udp;
using namespace std;
using namespace tobiss;

int main()
{
  try
  {
    boost::asio::io_service io_service;

    udp::socket socket(io_service, udp::endpoint(udp::v4(), 9998));
    boost::asio::socket_base::broadcast bcast(true);
    socket.set_option(bcast);
    vector<double> v;
    vector<uint16_t> blocks;
    unsigned int packet_count = 0;

    ofstream eeg_file;
    eeg_file.open("net_test.csv");

    for (;;)
    {
      boost::array<int, 4096> recv_buf;
//       udp::endpoint remote_endpoint;
      boost::system::error_code error;
      size_t len = socket.receive(boost::asio::buffer(recv_buf));
//       size_t len = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);

      if (error && error != boost::asio::error::message_size)
        throw boost::system::system_error(error);

      DataPacket packet(&recv_buf);

      v = packet.getData();
      blocks = packet.getNrOfBlocks();

      for(unsigned int n = 0; n < blocks.at(0); n++)
        eeg_file << v[n] << ", ";

      packet_count++;

      cout << "Got Packet! -- Nr SigTypes: " << packet.getNrOfSignalTypes() << endl;

//       cout << "EEG: " << packet.getSingleDataBlock(SIG_EEG) << endl;
      cout << "UserType 1: " << packet.getSingleDataBlock(SIG_USER_1)[0] << endl;


    }
    eeg_file.close();
    cout << endl << "  --> got " << packet_count << " packets"<<endl;

  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
