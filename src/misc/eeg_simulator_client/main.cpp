#include <iostream>
#include <sstream>
#include <string>

#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

//#include "../../../include/hardware/eeg_sim_msg_parser.h"
#include "eeg_sim_msg_parser.h"

using namespace std;
using namespace tobiss;

//-----------------------------------------------------------------------------

void setSineConfigInMultimap(boost::uint16_t ch,
                             std::multimap<boost::uint16_t,EEGSimMsgParser::SineConfig>& sine_map,
                             EEGSimMsgParser::SineConfig& config)
{

  std::pair< std::multimap<boost::uint16_t,EEGSimMsgParser::SineConfig>::iterator,
             std::multimap<boost::uint16_t,EEGSimMsgParser::SineConfig>::iterator > range(sine_map.equal_range(ch));

  bool found = 0;
  for(std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig> ::iterator it(range.first);
      it != range.second; it++)
  {
    if(config.freq_ == it->second.freq_)
    {
      it->second = config;
      found = 1;
    }
  }

  if(!found)
    sine_map.insert(  make_pair(ch, config ) );
}

//-----------------------------------------------------------------------------

void plot(  std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig> eeg,
          std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig> sines)
{
  cout << endl;
  cout << "EEG Configuration:" << endl << endl;
  cout << "  ch    scaling    offset"  << endl;
  cout << "-------------------------"  << endl;
  for(std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig>::iterator it(eeg.begin());
      it != eeg.end(); it++)
  {
    cout.width(4);
    cout << (it->first)+1;
    cout.width(11);
    cout << it->second.scaling_;
    cout.width(10);
    cout << it->second.offset_ << endl;
  }
  cout << endl << "Sine Configuration:" << endl << endl;
  cout << "  ch    freq    amplitude    phase"  << endl;
  cout << "----------------------------------"  << endl;
  for(std::map<boost::uint16_t, EEGSimMsgParser::SineConfig>::iterator it(sines.begin());
      it != sines.end(); it++)
  {
    cout.width(4);
    cout << (it->first)+1;
    cout.width(7);
    cout << it->second.freq_;
    cout.width(12);
    cout << it->second.amplitude_;
    cout.width(10);
    cout << it->second.phase_ << endl;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
  std::string ip = "127.0.0.1";
  int port = 9999;

  if(argc == 3)
  {
    ip = argv[1];
    stringstream conv(argv[2]);
    conv >> port;
    cout << "Using EEG Simulator: " << ip << ":" << port << endl;
  }

  boost::asio::io_service  io;
  std::string str_buffer;
  boost::asio::streambuf buf;
  EEGSimMsgParser parser(str_buffer);

  boost::asio::ip::tcp::socket socket(io);
  boost::asio::ip::address_v4 addr;
  addr.from_string(ip);
  boost::asio::ip::tcp::endpoint ep(addr, port);

  socket.connect(ep);
  boost::system::error_code  ec;
  socket.send(boost::asio::buffer("eegsimconfig:1:getconfig\n" ), 0, ec);
  boost::asio::read_until(socket, buf, '\n');
  str_buffer.clear();
  std::istream is(&buf);
  std::getline(is, str_buffer);
  is.get();

  parser.parseMessage();

  std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig> eeg_orig;
  std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig> sines_orig;
  parser.getConfigs(eeg_orig, sines_orig);

  std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig> eeg;
  std::multimap<boost::uint16_t, EEGSimMsgParser::SineConfig> sines;
  parser.getConfigs(eeg, sines);


  plot(eeg, sines);

  string str;
  cout << endl << ">>";

  while(cin >> str)
  {
    if(str == "q" || str == "quit" || str == "exit")
    {
      break;
    }


    else if(str == "p" || str == "plot")
    {
      plot(eeg, sines);
    }

    else if(str == "o" || str == "orig")
    {
      socket.send(boost::asio::buffer( parser.buildConfigMsgString(eeg_orig, sines_orig) ), 0, ec);
      eeg = eeg_orig;
      sines = sines_orig;
    }


    else if(str == "h" || str == "help")
    {
      cout << "q (quit)    ... Quit the signalserver" << endl;
      cout << "o (orig)    ... Send the original configuration to the EEG simulator" << endl;
      cout << "p (plot)    ... Plot the current configuration" << endl;
      cout << "e (eeg)     ... Edit the EEG configuration with 'ch/scaling/offset' ('a' for channel sets all channels)" << endl;
      cout << "s (sine)    ... Edit the Sine configuration with 'ch/freq/amplitude/phase' ('a' for channel sets all channels)" << endl;
      cout << "h (help)    ... Print this help" << endl;
      cout << endl << ">>";
    }


    else if(str == "e" || str == "eeg")
    {
      cout << endl << "(ch/scaling/offset) >> ";
      cin >> str; // ch,scaling,offset
      boost::regex expr_ch("\\d+/\\d+(.??\\d*)/\\d+(.??\\d*)");
      boost::regex expr_all("a/\\d++(.??\\d*)/\\d++(.??\\d*)");

      int ch = -1;
      int pos = 0;

      if(boost::regex_match(str, expr_ch))
      {
        pos = str.find('/');
        ch = boost::lexical_cast<int>(str.substr(0,pos)) -1;
        str.erase(0,pos+1);
      }
      else if(boost::regex_match(str, expr_all))
      {
        pos = str.find('/');
        str.erase(0,pos+1);
      }
      else
        cout << "Failure parsing input -- use following structure: 'ch/scaling/offset' " <<endl;

      pos = str.find('/');
      double scale = boost::lexical_cast<double>(str.substr(0,pos));
      str.erase(0,pos+1);

      double offset = boost::lexical_cast<double>(str);
      str.clear();
      if(ch >= 0)
      {
        if(eeg.find(ch) == eeg.end())
        {
          cerr << "Error -- Channel not set!" << endl;
          cout << endl << ">>";
          continue;
        }
        eeg[ch].scaling_ = scale;
        eeg[ch].offset_ = offset;
      }
      else
      {
        for(std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig>::iterator it(eeg.begin());
            it != eeg.end(); it++)
        {
          it->second.scaling_ = scale;
          it->second.offset_ = offset;
        }
      }
      socket.send(boost::asio::buffer( parser.buildConfigMsgString(eeg, sines) ), 0, ec);
    }


    else if(str == "s" || str == "sine")
    {
      cout << endl << "(ch/freq/amplitude/phase) >> ";
      cin >> str; // ch,freq,amplitude,phase

      boost::regex expr_ch("\\d+/\\d+(.??\\d*)/\\d+(.??\\d*)/\\d+(.??\\d*)");
      boost::regex expr_all("a/\\d++(.??\\d*)/\\d++(.??\\d*)/\\d+(.??\\d*)");

      int ch = -1;
      int pos = 0;

      if(boost::regex_match(str, expr_ch))
      {
        pos = str.find('/');
        ch = boost::lexical_cast<int>(str.substr(0,pos)) -1;
        str.erase(0,pos+1);
      }
      else if(boost::regex_match(str, expr_all))
      {
        pos = str.find('/');
        str.erase(0,pos+1);
      }
      else
        cout << "Failure parsing input -- use following structure: 'ch/scaling/offset' " <<endl;

      pos = str.find('/');
      double freq = boost::lexical_cast<double>(str.substr(0,pos));
      str.erase(0,pos+1);

      pos = str.find('/');
      double amplitude = boost::lexical_cast<double>(str.substr(0,pos));
      str.erase(0,pos+1);

      double phase = boost::lexical_cast<double>(str);
      str.clear();

      EEGSimMsgParser::SineConfig cfg;
      cfg.amplitude_ = amplitude;
      cfg.freq_ = freq;
      cfg.phase_ = phase;

      if(ch >= 0)
      {
        if(eeg.find(ch) == eeg.end())
        {
          cerr << "Error -- Channel not set!" << endl;
          cout << endl << ">>";
          continue;
        }
        setSineConfigInMultimap(ch,sines,cfg);
      }
      else
      {
        for(std::map<boost::uint16_t, EEGSimMsgParser::EEGConfig>::iterator it(eeg.begin());
            it != eeg.end(); it++)
        {
          setSineConfigInMultimap(it->first,sines,cfg);
        }
      }
      socket.send(boost::asio::buffer( parser.buildConfigMsgString(eeg, sines) ), 0, ec);
    }


    else
      cout << "Command '" << str << "' not recognized -- type 'help' for all possible commands!" <<endl;

    cout << endl << ">>";
  }

  socket.send(boost::asio::buffer( parser.buildConfigMsgString(eeg_orig, sines_orig) ), 0, ec);
  socket.close();
}
