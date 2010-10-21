
/*! \mainpage TOBI SignalServer v0.2
*
* \section sec_intro Introduction
* The TOBI SignalServer implements the data acquisition section from the TOBI hybrid BCI (hBCI).
* It also implements the so called TOBI Interface A within the class DataPacket.
*
* Up to now this project is in a very early stage of development and several errors might occur.
*
*
* \section sec_install Installation
* Information concerning the installation process will be provided soon.
* Up to now compilation of this project is based on qmake.
* Needed libraries are:
*   - boost-libs
*   - ticpp (tinyXML fpr C++)
*
*
* \section sec_notes Notes, to-do list etc.
* Tests have been performed using Ubuntu 9.10 and gcc 4.4.1.
*
* To use the g.tec g.USBamp, Microsofts Visual Studio compiler has to be used, otherwise
* acquiering data through this device will lead the program to crash.
*
*/

//-----------------------------------------------------------------------------

// STL
#include <iostream>

// Boost
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/exception/all.hpp>

// local
#include "signalserver/signal_server.h"
#include "signalserver/ssmethods.h"
#include "config/xml_parser.h"
#include "hardware/hw_access.h"
#include "filereading/data_file_handler.h"

using namespace std;

using namespace tobiss;

const string DEFAULT_XML_CONFIG = "server_config.xml";
const string XML_FILE_ARGUMENT = "-f";
const string DEAMON_ARGUMENT = "-d";
const int KEEP_ALIVE_TIMER = 50;

class DataPacketReader
{
  public:
    DataPacketReader(HWAccess& hw_access, SignalServer& server) :
        hw_access_(hw_access),
        server_(server),
        stop_reading_(false)
    {}

    void stop()
    {
      stop_reading_ = true;
    }

    void readPacket()
    {
      while (!stop_reading_)
      {
//         cout << "waiting... ";
        cout.flush();
        DataPacket p = hw_access_.getDataPacket();
//         cout << " got packet :-) ";
        server_.sendDataPacket(p);
//         cout << "packet, ";
      }
    }
  private:
    HWAccess&                   hw_access_;
    SignalServer&               server_;
    bool                        stop_reading_;
};

//-----------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
  try
  {
    string config_file;
    bool running = true;
    bool first_run = true;

    if(argc == 1)
    {
      config_file = DEFAULT_XML_CONFIG;
      cout << endl << " ***  Loading default XML configuration file: " << DEFAULT_XML_CONFIG << endl << endl;
    }
    else if(argc == 3 && argv[1] == XML_FILE_ARGUMENT)
    {
      cout << endl << "  ***  Loading XML configuration file: " << argv[2] << endl << endl;
      config_file = argv[2];
    }
    else if(argc == 2 && argv[1] == DEAMON_ARGUMENT)
    {
      config_file = DEFAULT_XML_CONFIG;
      cout << endl << "  ***  Server started in deamon-mode";
      cout << endl << "  ***  Loading XML configuration file: " << DEFAULT_XML_CONFIG << endl << endl;
    }
    else
      cout << " ERROR -- Wrong Number of input arguments!" << endl;

    XMLParser config(config_file);
    XMLParser temp_config;

    SSMethods* ss_methods = 0;

    while(running)
    {
      boost::asio::io_service io_service;

      SignalServer server(io_service);
//      server.setTimeoutKeepAlive(KEEP_ALIVE_TIMER);
      if(argc == 2 && argv[1] == DEAMON_ARGUMENT)
        server.setDeamonMode();

      if(!first_run)
      {
        config = temp_config;
      }
      else
      {
        first_run = false;
      }

      DataFileHandler data_file_handler(io_service, config.getFileReaderMap());

      HWAccess hw_access(io_service, config);

      DataPacketReader reader(hw_access, server);

      boost::thread* io_thread_ptr = 0;
      boost::thread* data_reader_thread_ptr = 0;

      if(config.usesDataFile())
      {
        // get DataPackets from FileReader and give it to the networking part
      }
      else
      {
        ss_methods = new SSMethods(&server, &config, &hw_access);
        ss_methods->startServerSettings();
        server.getConfig(temp_config);

        io_thread_ptr  = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));
        data_reader_thread_ptr = new boost::thread(boost::bind(&DataPacketReader::readPacket, &reader));

        #ifdef WIN32
          SetPriorityClass(io_thread_ptr->native_handle(), REALTIME_PRIORITY_CLASS);
          SetThreadPriority(io_thread_ptr->native_handle(), THREAD_PRIORITY_TIME_CRITICAL );
          SetPriorityClass(data_reader_thread_ptr->native_handle(), REALTIME_PRIORITY_CLASS);
          SetThreadPriority(data_reader_thread_ptr->native_handle(), THREAD_PRIORITY_TIME_CRITICAL );
        #endif
      }

      #ifdef WIN32
        SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
      #endif

      string str;
      cout << endl << ">>";

      while(cin >> str)
        if(str == "q")
        {
          running = false;
          break;
        }
        else if(str == "r")
        {
          break;
        }
        else
          cout << endl << ">>";

      reader.stop();
      io_service.stop();
      hw_access.stopDataAcquisition();

      if(io_thread_ptr)
      {
        io_thread_ptr->join();
        delete io_thread_ptr;
      }
      if(data_reader_thread_ptr)
      {
        data_reader_thread_ptr->join();
        delete data_reader_thread_ptr;
      }
      if(ss_methods)
      {
        delete ss_methods;
      }

      if(running)
      {
        cout << endl;
        cout << "   ...  Restarting and reloading SignalServer!" << endl;
        cout << endl;
        server.getConfig(temp_config);
        boost::this_thread::sleep(boost::posix_time::seconds(1));
      }
      else
      {
        // TODO: Notify all clients that server stops now
        cout << "Server stopped!" << endl;
      }
    }
  }
  catch(ticpp::Exception& ex)
  {
   cerr << endl << " ***** TICPP Exception caught! *****" << endl;
   cerr << " --> " << ex.what() << endl << endl;
  }
  catch(std::invalid_argument& e)
  {
   cerr << endl << " ***** STL Exception -- Invalid argument -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
  }
  catch(std::length_error& e)
  {
   cerr << endl << " ***** STL Exception -- Length error -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
  }
  catch(std::logic_error& e)
  {
   cerr << endl << " ***** STL Exception -- Logic error -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
  }
  catch(std::range_error& e)
  {
   cerr << endl << " ***** STL Exception -- Range error -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
  }
  catch(std::runtime_error& e)
  {
   cerr << endl << " ***** STL Exception -- Runtime error -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
  }
  catch(std::exception& e)
  {
   cerr << endl << " ***** STL Exception caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
  }
  catch(boost::exception& e)
  {
   cerr << endl << " ***** Boost Exception caught! *****" << endl;
   cerr << " --> " << boost::diagnostic_information(e) << endl << endl;
  }
  catch(...)
  {
   cerr << endl << " ***** Caught unknown exception! *****" << endl;
  }

   cerr.flush();
   return(0);
}

//-----------------------------------------------------------------------------
