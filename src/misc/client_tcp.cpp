
#include <iostream>
#include <fstream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <bitset>

#include "hardware/data_packet.h"

using boost::asio::ip::tcp;
using namespace std;

int main()
{

//       ofstream debug_file;
//       debug_file.open("debug.log");
//       streambuf* strm_buffer = cout.rdbuf();
//       cout.rdbuf(debug_file.rdbuf());

  try
  {
    boost::asio::io_service io_service;
    boost::asio::ip::address_v4 peer_ip;
//     peer_ip.from_string( string("127.0.0.1") );
    peer_ip.from_string( string("129.27.228.41") );
//     peer_ip = peer_ip.from_string("129.27.145.114");

    tcp::socket socket(io_service, tcp::endpoint(tcp::v4(), 0));

    cout << "Addr: " << peer_ip << endl;

    boost::asio::socket_base::receive_buffer_size option(4194304);
    socket.set_option(option);

    tcp::endpoint peer_endpoint(peer_ip, 9998);

    socket.connect(peer_endpoint);


    vector<double> v;
    vector<uint16_t> blocks;
    unsigned int packet_count = 0;
    unsigned int packet_nr = 0;
    unsigned int packet_size = 0;
    unsigned int packet_offset = 0;
    unsigned int buffered_data_size = 0;
    unsigned int write_offset = 0;

    size_t len = 0;

    ofstream eeg_file;
    eeg_file.open("net_test.csv");

    cout.unsetf(ios_base::showpos);
    cout.unsetf(ios_base::showbase);
    boost::array<char, 131072> recv_buf;
//     boost::array<char, 2048> recv_buf;

    for (;;)
    {
      boost::system::error_code error;

      if(!(packet_offset) )
      {
//         cout << "Network -- write offset: " << write_offset << ", packet offset: " << packet_offset << endl;
//         len = socket.receive(boost::asio::buffer(recv_buf));
//       size_t len = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);

	len = socket.receive( boost::asio::buffer( *( (boost::array<char, 131072>*)(  (char*)&recv_buf + write_offset ) ) ));

// len = socket.receive( boost::asio::buffer( *( (boost::array<char, 2048>*)(  (char*)&recv_buf + write_offset ) ) ));

// len = read(socket, boost::asio::buffer( *( (boost::array<char, 131072>*)(  (char*)&recv_buf + write_offset ) ) ) ,
//                    boost::asio::transfer_at_least(164));
        write_offset = 0;
        buffered_data_size += len;

      }

      if (error && error != boost::asio::error::message_size)
        throw boost::system::system_error(error);

      cout << "Got packet -- length: ";  cout.width(5);  cout.fill(' ');  cout << right << len << ", ";
      DataPacket* packet = 0;

      try
      {
        cout << "offset: " << packet_offset;
        cout << ", buffered data size: " << buffered_data_size;

        packet = new DataPacket(reinterpret_cast<char*>(&recv_buf) + packet_offset);
        packet_size = packet->getRequiredRawMemorySize();

        packet_offset += packet_size;

        cout << ", PacketSize: " << packet_size << ", New PacketOffset: " << packet_offset;
        cout << ", Remaining: "<< (int32_t)(buffered_data_size - packet_offset) << endl;

        if(packet_offset == buffered_data_size)
        {
          packet_offset = 0;
          buffered_data_size = 0;
          write_offset = 0;
        }
        else  // if( (int32_t)(buffered_data_size - packet_offset) < (int32_t)(packet_size))
        {

          uint32_t next_packet_size = packet->getRequiredRawMemorySize(reinterpret_cast<char*>(&recv_buf) + packet_offset,
                                      (int32_t)(buffered_data_size - packet_offset));

          if( next_packet_size == 0 || (int32_t)(buffered_data_size - packet_offset) < (int32_t)(next_packet_size))
          {
            write_offset = buffered_data_size - packet_offset;
            for(unsigned int n = 0; n < write_offset ; n++ )
              recv_buf[n] = recv_buf[n + packet_offset];

            packet_offset = 0;
            buffered_data_size = write_offset;
          }
        }

        cout << "OK, Nr:  ";
        cout.width(6);
        cout.fill(' ');
        cout << right << packet->getSampleNr() << ",  ";

//         uint64_t shift = 1;
//         for(unsigned int n = 0; n < 64; n++)
//         {
//           cout << (( *(uint64_t*)(&recv_buf) & shift) > 0);
//           shift <<= 1;
//           if(n == 15 || n == 31 || n == 47)
//             cout  << " ";
//         }
//         cout << endl;
//         cout.flush();

      }
      catch(std::runtime_error& e)
      {

        cout << "ERR, Nr: ";
        cout.width(6);
        cout.fill(' ');
        cout << right << packet->getSampleNr() << ",  ";
//         cout << left << "Got packet -- length: " << len << ",  ";
        uint64_t shift = 1;
        for(unsigned int n = 0; n < 64; n++)
        {
          cout << right << (( *(uint64_t*)(&recv_buf) & shift) > 0);
          shift <<= 1;
          if(n == 15 || n == 31 || n == 47)
            cout  << " ";
        }

        shift = 1;
        cout  << "  ";
        for(unsigned int n = 0; n < 64; n++)
        {
          cout << right << (( *(uint64_t*)(&recv_buf +7) & shift) > 0);
          shift <<= 1;
          if(n == 15 || n == 31 || n == 47)
            cout  << " ";
        }

        shift = 1;
        cout  << "  ";
        for(unsigned int n = 0; n < 64; n++)
        {
          cout << right << (( *(uint64_t*)(&recv_buf +15) & shift) > 0);
          shift <<= 1;
          if(n == 15 || n == 31 || n == 47)
            cout  << " ";
        }

        cout << endl;
        cout.flush();

        string ex_str(" ***** STL Exception -- Runtime error -- caught! *****\n  -->");
        ex_str += e.what();
        cout << endl << ex_str << endl;
        packet = new DataPacket( (&recv_buf) +16);
      }

      if(packet->getSampleNr() < packet_nr)
      {
        cout << "F*ck -- got a packet twice!" << endl;
        break;
      }

      packet_nr = packet->getSampleNr();

      v = packet->getData();
      blocks = packet->getNrOfBlocks();

//       for(unsigned int n = 0; n < blocks.at(0); n++)
//         cout << v[n] << ", ";
//         eeg_file << v[n] << ", ";


      double dbl = v[v.size()-1];

      cout << "Value: " << dbl << ";  ";
      unsigned long* ulp = reinterpret_cast<unsigned long*>(&dbl);
      bitset<64> bits1(*ulp);
      bitset<64> bits2(*(++ulp));
      bits2 <<= 32;    // Shift values left
      bits2 |= bits1;  // Combine this part with other part
      cout << bits2 << endl;

      if(dbl > 1.05 || dbl < -1.05 )
      {
        eeg_file.close();
        socket.close();

        return 1;
      }

      packet_count++;

      delete packet;
    }
    eeg_file.close();
    socket.close();
    cout << endl << "  --> got " << packet_count << " packets"<<endl;

  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
