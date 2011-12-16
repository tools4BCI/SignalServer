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

/**
* @file main.cpp
**/

//-----------------------------------------------------------------------------

// STL
#include <iostream>

// Boost
#include <boost/thread/thread.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

// local
#include "version.h"
#include "config/xml_parser.h"
#include "signalserver/signalserver.h"

using namespace std;
using namespace tobiss;
namespace po = boost::program_options;

const string DEFAULT_XML_CONFIG = "server_config.xml";
const string COMMENTS_XML_CONFIG = "server_config_comments.xml";

const string XML_CONFIG_FILE_PARAM = "server-config";
const string LIST_HARDWARE_PARAM   = "list-hardware";
const string OLD_TIA_PARAM = "use-old-tia";
const string HELP_PARAM = "help";

#ifndef WIN32
  const string DEFAULT_XML_CONFIG_HOME_SUBDIR = string("/tobi_sigserver_cfg/");
  const string TEMPLATE_XML_CONFIG = string("/usr/share/signalserver/") + DEFAULT_XML_CONFIG;
  const string TEMPLATE_XML_CONFIG_COMMENTS = string("/usr/share/signalserver/") + COMMENTS_XML_CONFIG;
#endif

template<class T> ostream& operator<<(ostream& os, const vector<T>& v);
string getDefaultConfigFile ();
void printPossibleHardware();
void printVersion();
int parseInput(int argc, const char* argv[], string& config_file, bool& use_new_tia);
void printRuntimeCommands();


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
  try
  {
    printVersion();

    string config_file;
    bool use_new_tia = true;

    if( parseInput(argc, argv, config_file, use_new_tia) )
      return 0;

    bool running = true;
    while(running)
    {
      XMLParser config(config_file);


      tobiss::SignalServer sig_server(config, use_new_tia);
      boost::thread* sig_server_ptr = 0;

      sig_server_ptr = new boost::thread(boost::bind(&SignalServer::readPackets, &sig_server));

      #ifdef WIN32
        SetPriorityClass(sig_server_ptr->native_handle(),  HIGH_PRIORITY_CLASS);
        SetThreadPriority(sig_server_ptr->native_handle(), THREAD_PRIORITY_HIGHEST );
        SetPriorityClass(GetCurrentProcess(),              REALTIME_PRIORITY_CLASS);
      #endif

      string str;
      cout << endl << ">>";

      while(cin >> str)
      {
        if(str == "q" || str == "quit" || str == "exit")
        {
          running = false;
          break;
        }
        else if(str == "r")
        {
          break;
        }
        else if(str == "h" || str == "help")
        {
          printRuntimeCommands();
          cout << endl << ">>";
        }
        else if(str == "l" || str == "list")
        {
          printPossibleHardware();
          cout << endl << ">>";
        }
        else
          cout << endl << ">>";
      }

      sig_server.stop();
      if(sig_server_ptr)
      {
        sig_server_ptr->join();
        delete sig_server_ptr;
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


std::string getDefaultConfigFile ()
{
    string default_xml_config = DEFAULT_XML_CONFIG;
#ifdef WIN32
    // do nothing here at the moment ;)
    return default_xml_config;
#else
    default_xml_config = string (getenv("HOME")) + DEFAULT_XML_CONFIG_HOME_SUBDIR + default_xml_config;
    string comments_xml_config = string (getenv("HOME")) + DEFAULT_XML_CONFIG_HOME_SUBDIR + COMMENTS_XML_CONFIG;
    boost::filesystem::path default_config_path (default_xml_config);
    boost::filesystem::path comments_config_path (comments_xml_config);

    boost::filesystem::path template_config_path (TEMPLATE_XML_CONFIG);
    boost::filesystem::path template_comments_config_path (TEMPLATE_XML_CONFIG_COMMENTS);

    if (!boost::filesystem::exists (default_config_path))
    {
        if (boost::filesystem::exists (template_config_path))
        {
            boost::filesystem::create_directory (default_config_path.parent_path());
            boost::filesystem::copy_file (template_config_path, default_config_path);
        }
        if (boost::filesystem::exists (TEMPLATE_XML_CONFIG_COMMENTS))
          boost::filesystem::copy_file (template_comments_config_path, comments_config_path);

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
  cout << "Important: This software comes with ABSOLUTELY NO WARRANTY, to the extent ";
  cout << "permitted by applicable law." << endl;
}

//-----------------------------------------------------------------------------

void printPossibleHardware()
{
  cout << "Possible hardware abrevations to be used  with this signal server version:" << endl;
  vector<string> hw_names = SignalServer::getPossibleHardwareNames();
  for(unsigned int n = 0; n < hw_names.size(); n++)
    cout << "  * " << hw_names[n] << endl;
}

//---------------------------------------------------------------------------------------

template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
  std::copy(v.begin(), v.end(), ostream_iterator<T>(cout, " "));
    return os;
}

//-----------------------------------------------------------------------------

int parseInput(int argc, const char* argv[], std::string& config_file, bool& use_new_tia)
{
  po::options_description desc("Allowed options");

  string str(HELP_PARAM +            ",h");
  desc.add_options() (str.c_str() ,  "   ... produce help message");

  str = LIST_HARDWARE_PARAM +   ",l";
  desc.add_options() (str.c_str() ,  "   ... list supported hardware");

  str = OLD_TIA_PARAM +         ",o";
  desc.add_options() (str.c_str() ,  "   ... use old version of TiA (undocumented)");

  str = XML_CONFIG_FILE_PARAM + ",f";
  desc.add_options() (str.c_str() ,  po::value< string >() , "   ... signal server config file path");

  po::positional_options_description p;
  p.add(XML_CONFIG_FILE_PARAM.c_str(), -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, const_cast<char **>(argv)).options(desc).positional(p).run(), vm);
  po::notify(vm);

  if(vm.count(HELP_PARAM))
  {
    cout << endl << "Usage: " << argv[0] << " [options]\n" << desc;
    cout << endl << "Runtime Commands: " << endl;
    printRuntimeCommands();
    return 1;
  }

  if(vm.count( LIST_HARDWARE_PARAM ))
  {
    printPossibleHardware();
    return 1;
  }

  if(vm.count( OLD_TIA_PARAM ))
  {
    use_new_tia = false;
    cout << endl << " ***  Signal Server will start with TiA 0.1 ***" << endl;
    vm.erase( OLD_TIA_PARAM );
  }
  else
    cout << endl << " ***  Signal Server will start with TiA 1.0 ***" << endl;

  if(vm.size() == 0)
  {
    config_file = getDefaultConfigFile ();
    cout << " ***  Loading default XML configuration file: " << config_file << endl << endl;
  }

  if(vm.count( XML_CONFIG_FILE_PARAM ))
  {
    config_file = vm[ XML_CONFIG_FILE_PARAM ].as< string >();
    cout << " ***  Loading XML configuration file: " << config_file << endl << endl;
  }
  return 0;
}

//-----------------------------------------------------------------------------

void printRuntimeCommands()
{
  cout << " q (quit)    ... Quit the signalserver" << endl;
  cout << " r (restart) ... Restart the signalserver (WARNING: malfunction with certain amplifiers possible)" << endl;
  cout << " l (list)    ... List possible hardware devices" << endl;
  cout << " h (help)    ... Print this help" << endl;
}

//-----------------------------------------------------------------------------

//    cout << "  -- Input files are: " << vm["input-files"].as< vector<string> >() << endl;
//    if(argc == 1)
//    {
//      config_file = getDefaultConfigFile ();
//      cout << endl << " ***  Loading default XML configuration file: " << config_file << endl << endl;
//    }
//    else if(argc == 2)
//    {
//      if(argv[1] == LIST_HARDWARE_PARAM)
//      {
//        printPossibleHardware();
//        return(0);
//      }
//      else if (argv[1] == NEW_TIA_PARAM)
//      {
//        use_new_tia = true;
//        cout << endl << " *** Signal Server will start with TiA 1.0 ***" << endl;
//        config_file = getDefaultConfigFile ();
//        cout << endl << " ***  Loading default XML configuration file: " << config_file << endl << endl;
//      }
//      else
//      {
//        cout << endl << "  ***  Loading XML configuration file: " << argv[1] << endl << endl;
//        config_file = argv[1];
//      }
//    }
//    else if(argc == 3 && argv[1] == XML_CONFIG_FILE_PARAM)
//    {
//      cout << endl << "  ***  Loading XML configuration file: " << argv[2] << endl << endl;
//      config_file = argv[2];
//    }
//    else
//      throw(std::invalid_argument(" ERROR -- Failure parsing input arguments!") );

