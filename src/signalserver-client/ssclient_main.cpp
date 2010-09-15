/**
* @ssclient_main.cpp
*
* @brief \TODO.
*
**/

// STL
#include <iostream>

// Boost
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>

// local
#include "datapacket/data_packet.h"
#include "signalserver-client/ssclient.h"
#include "signalserver-client/ssclientimpl.h"
#include "config/control_message_decoder.h"

using namespace std;
using namespace tobiss;

const string CONFIG_FILE_ARGUMENT = "-c";
const int KEEP_ALIVE_TIMER = 50;

//-----------------------------------------------------------------------------

class SSClientDataReader
{
  public:
    SSClientDataReader(SSClient& client, boost::mutex& mutex, boost::condition_variable& cond) :
        client_(client),
        mutex_(mutex),
        cond_(cond),
        running_(1),
        timestamp_(boost::posix_time::microsec_clock::local_time()),
        t_var_(0)
    {}

    void stop()
    {
      running_ = 0;
    }

    void readData()
    {
      unsigned int counter = 0;

      uint16_t nr_values = 0;
      uint64_t sample_nr = 0;
      uint64_t packet_nr = 0;

      uint64_t sample_nr_old = 0;
      uint64_t packet_nr_old = 0;

      while(running_)
      {
        {
          boost::unique_lock<boost::mutex> lock(mutex_);

          DataPacket packet;
          vector<double> v;

          if (!client_.receiving())
          {
            cond_.wait(lock);
          }

          if(running_ && client_.receiving())
          {
            try {
              client_.getDataPacket(packet);


              v = packet.getSingleDataBlock(1);
              nr_values = packet.getNrOfValues(1);

              sample_nr_old = sample_nr;
              packet_nr_old = packet_nr;

              sample_nr = packet.getSampleNr();
              packet_nr = packet.getPacketNr();

              if( (sample_nr - sample_nr_old) > 1)
              {
                cerr << "SampleNr difference: " << (sample_nr - sample_nr_old);
                cerr << " -- @Sample: " << sample_nr << endl;
              }

              if( (packet_nr - packet_nr_old) > 1)
              {
                cerr << "PacketNr difference: " << (packet_nr - packet_nr_old);
                cerr << " -- @Packet: " << packet_nr << endl;
              }

            }
            catch (std::exception& e)
            {
              cerr << e.what() << endl;
              continue;
            }

            #ifdef TIMING_TEST
              timestamp_ = boost::posix_time::microsec_clock::local_time();
              diff_ = timestamp_ - packet.getTimestamp();
              t_mean_ = (t_mean_ + diff_)/2;
              t_var_  = (t_var_ +
              ( (diff_.total_microseconds() - t_mean_.total_microseconds() )*
                (diff_.total_microseconds() - t_mean_.total_microseconds() )  ) )/2;
            #endif
          }
          else
            break;

          counter++;
          #ifdef TIMING_TEST
            if( ((client_.config().signal_info.masterSamplingRate()/client_.config().signal_info.masterBlockSize()) < 1) ||
              (counter%(
              (client_.config().signal_info.masterSamplingRate()/client_.config().signal_info.masterBlockSize()) *2 ) == 0) )
            {
              cout << "Packet Nr.: " << counter << ";  ";
              cout << "Timing -- mean: " << t_mean_.total_microseconds() << " microsecs,  ";
              cout << "variance: " << t_var_ << " microsecs"<< endl;
            }
          #endif
        }
      }
    }
  private:
    SSClient&                   client_;
    boost::mutex&               mutex_;
    boost::condition_variable&  cond_;
    bool                        running_;
    boost::posix_time::ptime timestamp_;
    boost::posix_time::time_duration diff_;
    boost::posix_time::time_duration t_mean_;
    boost::int64_t t_var_;
//     boost::posix_time::time_duration t_var_;
};

//-----------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
  string   srv_addr = "127.0.0.1";
  boost::uint16_t srv_port = 9000;
  std::string config_file;
  bool client_sends_config = false; //0 if server should make config, 1 if client should send config

  if(argc == 1)
  {
    cout << "Using default server " << srv_addr << ":" << srv_port << endl;
  }
  else if(argc == 3 && argv[1] == CONFIG_FILE_ARGUMENT)
  {
    cout << "Using default server " << srv_addr << ":" << srv_port << endl;
    cout << endl << "  ***  Loading own client-based XML configuration file: " << argv[2] << endl << endl;
    config_file = argv[2];
    client_sends_config = true;
  }
  else if(argc == 3 && argv[1] != CONFIG_FILE_ARGUMENT)
  {
    srv_addr = argv[1];
    stringstream conv(argv[2]);
    conv >> srv_port;
    cout << "Using server " << srv_addr << ":" << srv_port << endl;
  }
  else if(argc == 5 && argv[3] == CONFIG_FILE_ARGUMENT)
  {
    srv_addr = argv[1];
    stringstream conv(argv[2]);
    conv >> srv_port;
    cout << "Using server " << srv_addr << ":" << srv_port << endl;
    cout << endl << "  ***  Loading own client-based XML configuration file: " << argv[4] << endl << endl;
    config_file = argv[4];
    client_sends_config = true;
  }
  else
  {
    cout << "Wrong number of arguments given: " << argc-1 << endl;
    cout << " - Usage: " << argv[0] << "  [signalserver-ip   port] [-c config-file]" << endl;
    return(-1);
  }

  boost::asio::io_service io_service;
  SSClient client(io_service);

  client.setTimeoutKeepAlive(KEEP_ALIVE_TIMER);

  boost::thread* io_thread_ptr = 0;
  io_thread_ptr  = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));

  try
  {
    client.connect(srv_addr, srv_port);
    // Sleep because Client want to send without knowing if connected to Server
    // TODO: find another way to fix this
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    if(!client_sends_config)
    {
      client.requestConfig();
    }
    else
    {
      client.sendConfig(config_file);
    }
  }
  catch(std::exception& e)
  {
    cerr << e.what() << endl;
    delete io_thread_ptr;
    return 1;
  }

  boost::mutex mutex;
  boost::condition_variable cond;

  SSClientDataReader reader(client, mutex, cond);
  boost::thread reader_thread(boost::bind(&SSClientDataReader::readData, &reader));

    #ifdef WIN32
      SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
      SetPriorityClass(reader_thread.native_handle(), REALTIME_PRIORITY_CLASS);
      SetThreadPriority(reader_thread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL );
      SetPriorityClass(ka_reader_thread.native_handle(), REALTIME_PRIORITY_CLASS);
      SetThreadPriority(ka_reader_thread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL );
    #endif

  map<string, string> known_commands;
  known_commands.insert(make_pair("config",       "Requests the config from server."));
  known_commands.insert(make_pair("sendconfig",   "Sends the config to server."));
  known_commands.insert(make_pair("starttcp",     "Starts the data transmission using tcp"));
  known_commands.insert(make_pair("startudp",     "Starts the data transmission using udp broadcast"));
  known_commands.insert(make_pair("stop",         "Stops the data transmission"));
  known_commands.insert(make_pair("q",            "Quits the client."));
  known_commands.insert(make_pair("help",         "Prints this help text."));

  string command;
  cout << endl << ">>";

  while(cin >> command)
  {
    {
      map<string, string>::const_iterator it = known_commands.find(command);
      if (it == known_commands.end())
      {
        cout << "Type 'help' to get description of supported commands" << endl;
        cout << endl << ">>";
        continue;
      }
    }

//    boost::unique_lock<boost::mutex> lock(mutex);

    if(command == "q")
    {
      client.stopReceiving();
      reader.stop();
      io_service.stop();
      if(io_thread_ptr)
      {
        io_thread_ptr->join();
        delete io_thread_ptr;
      }
      cond.notify_all();

      reader_thread.join();
      if (client.receiving())
      {
        cerr << "Cannot Stop Receiving!" << endl;
      }
      break;
    }
    if (command == "config")
    {
      client.requestConfig();
    }
    else if (command == "sendconfig")
    {
      if((argv[1] == CONFIG_FILE_ARGUMENT || argv[3] == CONFIG_FILE_ARGUMENT))
      {
        client.sendConfig(config_file);
      }
      else
      {
        cerr << "Client not started with own config-file" << endl;
      }
    }
    else if (command == "starttcp" || command == "startudp")
    {
      bool udp =  command == "startudp";
      cout << "Start Receiving ..." << endl;
      client.startReceiving(udp);

      if (!client.receiving())
      {
        cerr << "Start Receiving failed" << endl;
      }

      cond.notify_one();
    }
    else if (command == "stop")
    {
      cout << "Stop Receiving ..." << endl;
      client.stopReceiving();

      if (client.receiving())
      {
        cerr << "Cannot Stop Receiving!" << endl;
      }
    }
    else if (command == "help")
    {
      map<string, string>::const_iterator it = known_commands.begin();
      map<string, string>::const_iterator end = known_commands.end();
      for (; it != end; ++it)
      {
        const string& command_name = (*it).first;
        string spacing;
        spacing.assign(20 - command_name.size(), '.');
        cout << (*it).first << spacing << (*it).second << endl;
      }
    }

    cond.notify_one();
    cout << endl << ">>";
  }

 return(0);
}

//-----------------------------------------------------------------------------
