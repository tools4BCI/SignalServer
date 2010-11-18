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
*   - boost-libs  (recommeneded: >=1.40)
*   - ticpp  (tinyXML fpr C++)
*   - SDL    (Simple Directmedia lLayer)
*   - libusb (v1.0)
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
#include <boost/filesystem.hpp>

// local
#include "version.h"
#include "signalserver/signal_server.h"
#include "config/xml_parser.h"
#include "hardware/hw_access.h"
#include "filereading/data_file_handler.h"

using namespace std;

using namespace tobiss;

const string DEFAULT_XML_CONFIG = "server_config.xml";
const string COMMENTS_XML_CONFIG = "server_config_comments.xml";
const string XML_CONFIG_FILE_PARAM = "-f";

#ifndef WIN32
const string DEFAULT_XML_CONFIG_HOME_SUBDIR = string("/tobi_sigserver_cfg/");
const string TEMPLATE_XML_CONFIG = string("/usr/local/etc/signalserver/") + DEFAULT_XML_CONFIG;
const string TEMPLATE_XML_CONFIG_COMMENTS = string("/usr/local/etc/signalserver/") + COMMENTS_XML_CONFIG;
#endif

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
string getDefaultConfigFile ()
{
    string default_xml_config = DEFAULT_XML_CONFIG;
#ifdef WIN32
    // do nothing here at the moment ;)
    return default_xml_config;
#else
    default_xml_config = string (getenv("HOME")) + DEFAULT_XML_CONFIG_HOME_SUBDIR + default_xml_config;
    boost::filesystem::path default_config_path (default_xml_config);
    boost::filesystem::path template_config_path (TEMPLATE_XML_CONFIG);
    boost::filesystem::path template_comments_config_path (TEMPLATE_XML_CONFIG_COMMENTS);

    if (!boost::filesystem::exists (default_config_path))
    {
        if (boost::filesystem::exists (template_config_path))
        {
            boost::filesystem::create_directory (default_config_path.parent_path());
            boost::filesystem::copy_file (template_config_path, default_config_path);
        }
        if (boost::filesystem::exists (template_config_path))
          boost::filesystem::copy_file (template_comments_config_path, default_config_path);

    }
    return default_xml_config;
#endif
}


//-----------------------------------------------------------------------------

void printVersion()
{
  cout << endl;
  cout << "SignalServer -- Version: " << MAJOR_VERSION;
  cout << " (build " << BUILD_NUMBER << ")";
  #ifndef WIN32
    cout << " -- " << BUILD_STR;
  #else
    cout << " -- " << __DATE__ << " " << __TIME__;
  #endif
  cout << endl << endl;
  cout << "Laboratory of Brain-Computer Interfaces" << endl;
  cout << "Graz University of Technology" << endl;
  cout << "http://bci.tugraz.at" << endl;
}

int main(int argc, const char* argv[])
{
  try
  {
    printVersion();

    string config_file;
    bool running = true;

    if(argc == 1)
    {
      config_file = getDefaultConfigFile ();
      cout << endl << " ***  Loading default XML configuration file: " << config_file << endl << endl;
    }
    else if(argc == 2)
    {
      cout << endl << "  ***  Loading XML configuration file: " << argv[1] << endl << endl;
      config_file = argv[1];
    }
    else if(argc == 3 && argv[1] == XML_CONFIG_FILE_PARAM)
    {
      cout << endl << "  ***  Loading XML configuration file: " << argv[2] << endl << endl;
      config_file = argv[2];
    }
    else
      throw(std::invalid_argument(" ERROR -- Failure parsing input arguments!") );

    while(running)
    {
      XMLParser config(config_file);


      boost::asio::io_service io_service;

      SignalServer server(io_service);

//      DataFileHandler data_file_handler(io_service, config.getFileReaderMap());

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
        // TODO: find a better way to pass this information to the server
        server.setMasterBlocksize(hw_access.getMastersBlocksize());
        server.setMasterSamplingRate(hw_access.getMastersSamplingRate());
        server.setAcquiredSignalTypes(hw_access.getAcquiredSignalTypes());
        server.setBlockSizesPerSignalType(hw_access.getBlockSizesPerSignalType());
        server.setSamplingRatePerSignalType(hw_access.getSamplingRatePerSignalType());
        server.setChannelNames(hw_access.getChannelNames());

        server.initialize(config.parseSubject(),config.parseServerSettings());
        hw_access.startDataAcquisition();

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
   std::cin.peek();
  }
  catch(std::invalid_argument& e)
  {
   cerr << endl << " ***** STL Exception -- Invalid argument -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
   std::cin.peek();
  }
  catch(std::length_error& e)
  {
   cerr << endl << " ***** STL Exception -- Length error -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
   std::cin.peek();
  }
  catch(std::logic_error& e)
  {
   cerr << endl << " ***** STL Exception -- Logic error -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
   std::cin.peek();
  }
  catch(std::range_error& e)
  {
   cerr << endl << " ***** STL Exception -- Range error -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
   std::cin.peek();
  }
  catch(std::runtime_error& e)
  {
   cerr << endl << " ***** STL Exception -- Runtime error -- caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
   std::cin.peek();
  }
  catch(std::exception& e)
  {
   cerr << endl << " ***** STL Exception caught! *****" << endl;
   cerr << " --> " << e.what() << endl << endl;
   std::cin.peek();
  }
  catch(boost::exception& e)
  {
   cerr << endl << " ***** Boost Exception caught! *****" << endl;
   cerr << " --> " << boost::diagnostic_information(e) << endl << endl;
   std::cin.peek();
  }
  catch(...)
  {
   cerr << endl << " ***** Caught unknown exception! *****" << endl;
   std::cin.peek();
  }

   cerr.flush();
   return(0);
}

//-----------------------------------------------------------------------------
