
/*! \mainpage TOBI SignalServer v0.1
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
#include "config/xml_parser.h"
#include "hardware/hw_access.h"

using namespace std;

using namespace tobiss;

const string DEFAULT_XML_CONFIG = "server_config.xml";
const string XML_FILE_ARGUMENT = "-f";

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
    else
      cout << " ERROR -- Wrong Number of input arguments!" << endl;

    while(running)
    {
      XMLParser config(config_file);

      boost::asio::io_service io_service;

      SignalServer server(io_service);

      HWAccess hw_access(io_service, config);

      // TODO: find a better way to pass this information to the server
      server.setMasterBlocksize(hw_access.getMastersBlocksize());
      server.setMasterSamplingRate(hw_access.getMastersSamplingRate());
      server.setAcquiredSignalTypes(hw_access.getAcquiredSignalTypes());
      server.setBlockSizesPerSignalType(hw_access.getBlockSizesPerSignalType());
      server.setSamplingRatePerSignalType(hw_access.getSamplingRatePerSignalType());
      server.setChannelNames(hw_access.getChannelNames());

      server.initialize(&config);
      hw_access.startDataAcquisition();

      boost::thread st(boost::bind(&boost::asio::io_service::run, &io_service));

      DataPacketReader reader(hw_access, server);
      boost::thread data_reader_thread(boost::bind(&DataPacketReader::readPacket, &reader));

      #ifdef WIN32
        SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
        SetPriorityClass(st.native_handle(), REALTIME_PRIORITY_CLASS);
        SetThreadPriority(st.native_handle(), THREAD_PRIORITY_TIME_CRITICAL );
        SetPriorityClass(data_reader_thread.native_handle(), REALTIME_PRIORITY_CLASS);
        SetThreadPriority(data_reader_thread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL );
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
      st.join();
      data_reader_thread.join();

      if(running)
      {
        cout << endl;
        cout << "   ...  Restarting and reloading SignalServer!" << endl;
        cout << endl;
        boost::this_thread::sleep(boost::posix_time::seconds(1));
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
