
/**
* @file test_hardware.cpp
*
* @brief File including Main() -- only for hardware testing purposes
*
* test_hardware.cpp tests data acquisition accessing hw_access. It tests
* configuration (hardware, server config,...) through an xml-file,
* gathering DataPackets and rebuilding them from a RAW representation.
*   ---  This file is not intended to be included in the final project version!  ---
**/

#include <iostream>
#include <fstream>

#include <vector>
#include <string>
#include <map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/exception.hpp>
#include <boost/thread.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include "ticpp/ticpp.h"
#include "xml_parser.h"
#include "hardware/hw_access.h"

	#ifdef WIN32
		#include <boost/cstdint.hpp>
	#else
		#include <stdint.h>
	#endif

using namespace std;
using namespace boost::posix_time;

const string DEFAULT_XML_CONFIG = "server_config.xml";
const string XML_FILE_ARGUMENT = "-f";

//---------------------------------------------------------------------------------------

void run(boost::asio::io_service* io)
{
  io->run();
}

//---------------------------------------------------------------------------------------

void showBinary (uint32_t flags)
{
  uint32_t shift = 1;

  for(unsigned int n = 0; n < 32; n++)
  {
    cout << ((flags & shift) > 0);
    shift <<= 1;
  }
}

//---------------------------------------------------------------------------------------

void showDataPacket(HWAccess* hw, int* running)
{
  unsigned int packet_count = 0;
  vector<double> v;
  vector<uint16_t> blocks;

  if(!(*running))
    return;

  ptime t(microsec_clock::local_time());
  cout << "Data Acquisition Started! -- Time: " << t << endl;
  cout << "WARNING: Displaying this text inc. timing information causes a 500 bytes leak or disturbes controlled process termination!!" << endl;

  cout << showpoint << showpos << fixed;
  while(*running)
  {
    DataPacket p = hw->getDataPacket();
    v = p.getData();
//     v = p.getSingleDataBlock(SIG_EEG);
    blocks = p.getNrOfBlocks();

    cout << " - Orig Packet:  ";
    //     for(unsigned int n = 0; n < v.size(); n++)
    for(unsigned int n = 0; n < blocks.at(0); n++)
      cout << v[n] << ", ";
    cout << endl;
    cout.flush();
//
//     DataPacket p_raw(p.getRaw());
//     v = p_raw.getData();
//     cout << " - Raw Packet:   ";
//     for(unsigned int n = 0; n < v.size(); n++)
//       cout << v[n] << ", ";
//     cout << endl;
//     cout.flush();
    packet_count++;
  }

  cout << endl << "  --> got " << packet_count << " packets"<<endl;
  cout << endl << "Data Acquisition Stopped! -- Time: " << microsec_clock::local_time();
  cout << " --> Difference: " << microsec_clock::local_time() -t << endl;
}

//---------------------------------------------------------------------------------------

void writePackets2File(HWAccess* hw, ofstream* file, int* running)
{
  unsigned int packet_count = 0;
  vector<double> v;
  vector<uint16_t> blocks;

  DataPacket p;

  if(!(*running))
    return;

  ptime t(microsec_clock::local_time());
  cout << "Data Acquisition Started! -- Time: " << t << endl;
  cout << "WARNING: Displaying this text inc. timing information causes a 500 bytes leak or disturbes controlled process termination!!" << endl;
  while(*running)
  {
    p = hw->getDataPacket();
    v = p.getData();
//     v = p.getSingleDataBlock(SIG_EEG);
    blocks = p.getNrOfBlocks();

    for(unsigned int n = 0; n < blocks.at(0); n++)
      *file << v[n] << ", ";

//     DataPacket p_raw(p.getRaw());
//     v = p_raw.getData();

    packet_count++;
  }

  cout << endl << "  --> got " << packet_count << " packets"<<endl;
  cout << endl << "Data Acquisition Stopped! -- Time: " << microsec_clock::local_time();
  cout << " --> Difference: " << microsec_clock::local_time() -t << endl;
  file->flush();
}

//---------------------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
  boost::asio::io_service io;
  map<string,string> subject_map;
  map<string,string> server_settings_map;
  XMLParser x;
  cout.precision(12);

  try
  {
    if(argc == 1)
    {
      cout << endl << " ***  Loading default XML configuration file: " << DEFAULT_XML_CONFIG << endl << endl;
      x = XMLParser(DEFAULT_XML_CONFIG);
    }
    else if(argc == 3 && argv[1] == XML_FILE_ARGUMENT)
    {
      cout << endl << "  ***  Loading XML configuration file: " << argv[2] << endl << endl;
      x = XMLParser(argv[2]);
    }
    else
      cout << " ERROR -- Wrong Number of input arguments!" << endl;

    subject_map = x.parseSubject();
    server_settings_map = x.parseServerSettings();

    int run_show = 0;
    int* s = &run_show;
    int run_write = 0;
    int* w = &run_write;

    char stop = 0;
    ofstream eeg_file;
    eeg_file.open("test.csv");

    char mode = 0;

    cout << endl << "Enter Mode: (s ... show packets,  w ... write packets to file)";

    while( (mode != 's') && (mode != 'w') && (mode != 'q'))
    {
      if((mode != 's') && (mode != 'w') && (mode != 'q') && (mode != 0) )
        cout << "Unknown character:  " << mode << endl;
      cin >> mode;
    }

    if(mode == 'q')
      return(0);

    HWAccess hw = HWAccess(io, x);
    hw.startDataAcquisition();



    boost::thread th_io(boost::bind(run,&io));

    if(mode == 's')
      run_show = 1;
    if(mode == 'w')
      run_write = 1;


    boost::thread th_show(boost::bind(showDataPacket, &hw, s));
    boost::thread th_write(boost::bind(writePackets2File, &hw, &eeg_file, w));

    #ifdef WINDOWS
     HANDLE th_s = th_show.native_handle();
     HANDLE th_w = th_write.native_handle();

     SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
     SetPriorityClass(th_io.native_handle(), REALTIME_PRIORITY_CLASS);
     SetPriorityClass(th_s, REALTIME_PRIORITY_CLASS);
     SetPriorityClass(th_w, REALTIME_PRIORITY_CLASS);

     SetThreadPriority(th_io.native_handle(), THREAD_PRIORITY_TIME_CRITICAL );
     SetThreadPriority(th_s, THREAD_PRIORITY_TIME_CRITICAL );
     SetThreadPriority(th_w, THREAD_PRIORITY_TIME_CRITICAL );
    #endif

    while(stop != 'q')
    {
      cin >> stop;
    }
    run_show = 0;
    run_write = 0;
    hw.stopDataAcquisition();

    io.stop();
    th_io.join();

    th_show.join();
    th_write.join();

    eeg_file.close();

    cout << endl;
    cout.flush();
    return(0);
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

//---------------------------------------------------------------------------------------
