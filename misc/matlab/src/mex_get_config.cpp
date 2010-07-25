
#include "mex.h"
#include "matrix.h"

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <fstream>

#include <boost/exception/all.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "ticpp/ticpp.h"

#include "definitions/constants.h"
#include "signalserver-client/ssclient.h"
#include "signalserver-client/ssconfig.h"

#include <boost/cstdint.hpp>

#define NUM_INPUTS    3
#define IP_POS        0
#define PORT_POS      1   // xml config msg port
#define PROTOCOL_POS  2

#define NUM_OUTPUTS   4
#define FS_BS_POS        0    //master sampling rate and blocksize
#define SIG_TYPES_POS    1    //signal types info (signal type (flag), signal type (string), blocksizes, nr of channels, fs)
    #define NR_SIG_TYPES_INFO   5
#define CH_INFO_POS      2    //channel information
    #define NR_CH_INFO   2
#define SERVER_INFO_POS      3    //server information
    #define NR_SERVER_INFO   1    // data_port
using boost::asio::ip::udp;
using boost::uint16_t;
using boost::uint32_t;

using namespace std;
using namespace tobiss;

const string DEFAULT_XML_CONFIG = "server_config.xml";

//---------------------------------------------------------------------------------------

void run(boost::asio::io_service* io)
{
  cout << "run: io service started in thread" << endl;
  io->run();
  cout << "run: io service ended" << endl;
}

//---------------------------------------------------------------------------------------

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
  uint32_t port;

  if(nrhs != NUM_INPUTS)  // IP, port, protocol
    mexErrMsgTxt("3 input arguments required.");
  if(nlhs != NUM_OUTPUTS)
    mexErrMsgTxt("4 output arguments required.");

  if ( !mxIsChar(prhs[IP_POS]) )
    mexErrMsgTxt("IP must be a string.");
  if ( !mxIsNumeric(prhs[PORT_POS]) )
    mexErrMsgTxt("Port must be a number.");
  if ( !mxIsChar(prhs[PROTOCOL_POS]) )
    mexErrMsgTxt("Protocol must be a string.");

  if (mxGetM(prhs[IP_POS]) !=1 )
    mexErrMsgTxt("IP must be a row vector.");
  if (mxGetM(prhs[PORT_POS]) !=1 &&  mxGetN(prhs[PORT_POS]) !=1)
    mexErrMsgTxt("Port must be a single number.");
  if (mxGetM(prhs[PROTOCOL_POS]) !=1 )
    mexErrMsgTxt("Protocol must be a row vector.");


  try
  {
    string srv_addr(mxArrayToString(prhs[IP_POS]));
    string protocol(mxArrayToString(prhs[PROTOCOL_POS]));
    port = mxGetScalar(prhs[PORT_POS]);

//     stringstream ss;
//     streambuf* strm_buffer = cout.rdbuf();
//     cout.rdbuf (ss.rdbuf());

    Constants cst;
//     cout.rdbuf (strm_buffer);

    cout << "Using server " << srv_addr << ":" << port << endl;
    SSClient client;
    client.connect(srv_addr, port);

    client.requestConfig();
    SSConfig config = client.config();
    tobiss::SignalInfo::SignalMap& signals(config.signal_info.signals());
    std::map<uint32_t, tobiss::Signal> sig_types_map;
    tobiss::SignalInfo::SignalMap::iterator iter(signals.begin());

    for( ; iter != signals.end(); iter++)
      sig_types_map.insert( make_pair(cst.getSignalFlag(iter->first), iter->second)  );

    std::map<uint32_t, tobiss::Signal>::iterator it(sig_types_map.begin());

//      cout << "Nr of SigTypes: " << signals.size() << endl;

    vector<uint32_t> sig_types;
    vector<uint16_t> blocksizes;
    vector<uint32_t> fs_per_sig_type;
    vector<uint16_t> ch_per_sig_type;

    map<uint32_t, vector<string> >  naming;
    uint32_t nr_ch = 0;

    for( ; it != sig_types_map.end(); it++)
    {
      sig_types.push_back( it->first );
      blocksizes.push_back(it->second.blockSize());
      fs_per_sig_type.push_back(it->second.samplingRate());
      ch_per_sig_type.push_back(it->second.channels().size());

      vector<string> names;
      for(unsigned int n = 0; n < it->second.channels().size(); n++ )
      {
        nr_ch++;
        names.push_back(it->second.channels()[n].id());
      }
      naming.insert( make_pair(it->first ,names) );
    }

    plhs[FS_BS_POS] = mxCreateDoubleMatrix(1,2, mxREAL);
    double* master_info = mxGetPr(plhs[FS_BS_POS]);
    master_info[0] = config.signal_info.masterSamplingRate();
    master_info[1] = config.signal_info.masterBlockSize();

    mxArray* sig_types_info = mxCreateCellMatrix(sig_types.size(), NR_SIG_TYPES_INFO);
    for(uint32_t n = 0; n < sig_types.size(); n++)
    {
      mxSetCell(sig_types_info, n, mxCreateDoubleScalar(sig_types[n]) );
      mxSetCell(sig_types_info, n + sig_types.size(), mxCreateString(cst.getSignalName(sig_types[n]).c_str()  ) );
      mxSetCell(sig_types_info, n + 2 * sig_types.size(), mxCreateDoubleScalar(blocksizes[n]) );
      mxSetCell(sig_types_info, n + 3 * sig_types.size(), mxCreateDoubleScalar(ch_per_sig_type[n]) );
      mxSetCell(sig_types_info, n + 4 * sig_types.size(), mxCreateDoubleScalar(fs_per_sig_type[n]) );

//       sig_types_info[n] = sig_types[n];
//       sig_types_info[ n + 2 * sig_types.size()] = blocksizes[n];
//       sig_types_info[ n + 3 * sig_types.size()] = ch_per_sig_type[n];
    }

    plhs[SIG_TYPES_POS] = sig_types_info;

    mxArray* ch_info  = mxCreateCellMatrix(nr_ch, NR_CH_INFO);
    uint32_t channel = 0;
    for(map<uint32_t, vector<string> >::iterator it(naming.begin()); it != naming.end(); it++)
      for(uint32_t n = 0; n < it->second.size(); n++)
      {
        mxSetCell(ch_info, channel, mxCreateString(it->second[n].c_str()));
        mxSetCell(ch_info, channel + nr_ch, mxCreateString( cst.getSignalName(it->first).c_str() ));
        channel++;
      }
    plhs[CH_INFO_POS]  = ch_info;

    plhs[SERVER_INFO_POS] = mxCreateDoubleScalar(port);

  }
  catch(ticpp::Exception& e)
  {
    string ex_str(" ***** TICPP Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::invalid_argument& e)
  {
    string ex_str(" ***** STL Exception -- Invalid argument -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::length_error& e)
  {
    string ex_str(" ***** STL Exception -- Length error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::logic_error& e)
  {
    string ex_str(" ***** STL Exception -- Logic error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::range_error& e)
  {
    string ex_str(" ***** STL Exception -- Range error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::runtime_error& e)
  {
    string ex_str(" ***** STL Exception -- Runtime error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::exception& e)
  {
    string ex_str(" ***** STL Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(boost::exception& e)
  {
    string ex_str(" ***** Boost Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(...)
  {
    mexErrMsgTxt("ERROR !!  --  Unknown exception caught!");
  }
    return;
}
