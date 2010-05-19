
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

#include "../ticpp/ticpp.h"

#include "../constants.h"
#include "../ssclient/ssclient.h"


#ifdef WIN32
#include <boost/cstdint.hpp>
#else
#include <stdint.h>
#endif

#define NUM_INPUTS_GETDATA   0
#define NUM_INPUTS_CLOSE     1
#define NUM_INPUTS_CONFIG    3
#define IP_POS        0
#define PORT_POS      1   // xml config msg port
#define PROTOCOL_POS  2

#define NUM_OUTPUTS   3
#define FS_BS_POS        0    //master sampling rate and blocksize
#define SIG_TYPES_POS    1    //signal types info (signal type (flag), signal type (string), blocksizes, nr of channels, fs)
    #define NR_SIG_TYPES_INFO   5
#define CH_INFO_POS      2    //channel information
    #define NR_CH_INFO   2
using boost::asio::ip::udp;
using namespace std;

const string DEFAULT_XML_CONFIG = "server_config.xml";
static SSClient* client = 0;
static std::map<uint32_t, tobiss::Signal> *sig_types_map;



//---------------------------------------------------------------------------------------

void run(boost::asio::io_service* io)
{
  cout << "run: io service started in thread" << endl;
  io->run();
  cout << "run: io service ended" << endl;
}

//---------------------------------------------------------------------------------------

void ssc_getConfig(int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]);
void ssc_close();
void ssc_getData(int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]);


void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
  if(NUM_INPUTS_GETDATA == nrhs) {
    ssc_getData(nlhs,plhs,nrhs,prhs);
  } else if(NUM_INPUTS_CLOSE == nrhs) {
    ssc_close();
  } else if(NUM_INPUTS_CONFIG == nrhs){
    ssc_getConfig(nlhs,plhs,nrhs,prhs);
  }
}
void ssc_getConfig(int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
  uint32_t port;

  if(nlhs != NUM_OUTPUTS) {
    mexErrMsgTxt("4 output arguments required.");
  }

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
    if(0 != client){
        mexErrMsgTxt("Client already running.");
    }else {
        client = new SSClient();
	sig_types_map = new std::map<uint32_t, tobiss::Signal>();
    }
    
    client->connect(srv_addr, port);

    client->requestConfig();
    SSConfig config = client->config();
    tobiss::SignalInfo::SignalMap& signals(config.signal_info.signals());
    
    tobiss::SignalInfo::SignalMap::iterator iter(signals.begin());

    for( ; iter != signals.end(); iter++)
      sig_types_map->insert( make_pair(cst.getSignalFlag(iter->first), iter->second)  );

    std::map<uint32_t, tobiss::Signal>::iterator it(sig_types_map->begin());

//      cout << "Nr of SigTypes: " << signals.size() << endl;

    vector<uint32_t> sig_types;
    vector<uint16_t> blocksizes;
    vector<uint32_t> fs_per_sig_type;
    vector<uint16_t> ch_per_sig_type;

    map<uint32_t, vector<string> >  naming;
    uint32_t nr_ch = 0;

    for( ; it != sig_types_map->end(); it++)
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

    client->startReceiving(false);

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

void ssc_getData(int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]) {
//   map<uint32_t, pair<uint16_t, uint16_t> >* sig_info =
  //static_cast<map<uint32_t, pair<uint16_t, uint16_t> >* >(ssGetPWork(S)[SIGNAL_TYPES_MAP_POSITION]);

  
  try
  {
   uint16_t nr_values = 0;
   vector<double> v;
   DataPacket packet;
 
   if (!client->receiving())
   {
     mexErrMsgTxt( "Receiving failed!");
   }

    client->getDataPacket(packet);
    mxArray* cellArray = mxCreateCellMatrix(packet.getNrOfSignalTypes(),2);
    plhs[0] = cellArray;
    
    unsigned int cellIndex = 0;
    for(map<uint32_t, tobiss::Signal >::iterator it = sig_types_map->begin();
	it != sig_types_map->end();  it++)
    {
      try {
	
	v = packet.getSingleDataBlock(it->first);
	int blockChannels = it->second.channels().size(); 
	int blockSamples = it->second.blockSize();
	
	mxArray* blockMatrix = mxCreateDoubleMatrix(blockSamples, blockChannels,mxREAL);
	double* blockMatrixValues = mxGetPr(blockMatrix);
	nr_values = packet.getNrOfValues(it->first);

	for(unsigned int n = 0; n < nr_values; n++) {
	  
	  blockMatrixValues[n] = v[n];
	}
	
	mxSetCell(cellArray, cellIndex, mxCreateString(it->second.type().c_str()));
	mxSetCell(cellArray, cellIndex + packet.getNrOfSignalTypes(), blockMatrix);
    
	cellIndex++;
      }
      catch(std::invalid_argument& e)
      {
      //     Exception from "p->getSingleDataBlock( flag )" if flag not found
      //       --> do nothing (don't write output port)
      }
      
    }

   //     get possible event
//    real_T *y = ssGetOutputPortRealSignal(S, port);
//    try
//    {
//      v = packet.getSingleDataBlock(SIG_EVENT);
//      nr_values = packet.getNrOfValues(SIG_EVENT);
// 
//      y[0] = nr_values;
// 
//      for(unsigned int n = 0; n < nr_values; n++)
//        y[n+1] = v[n];
//    }
//    catch(std::invalid_argument& e)
//    {
//      y[0] = 0;
//    }
  }
  catch(bad_numeric_cast& e)
  {
    string ex_str(" ***** Boost Numeric Cast Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(ticpp::Exception& e)
  {
    string ex_str(" ***** TICPP Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::invalid_argument& e)
  {
    string ex_str(" ***** STL Exception -- Invalid argument -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::length_error& e)
  {
    string ex_str(" ***** STL Exception -- Length error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::logic_error& e)
  {
    string ex_str(" ***** STL Exception -- Logic error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::range_error& e)
  {
    string ex_str(" ***** STL Exception -- Range error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::runtime_error& e)
  {
    string ex_str(" ***** STL Exception -- Runtime error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::exception& e)
  {
    string ex_str(" ***** STL Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(boost::exception& e)
  {
    string ex_str(" ***** Boost Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(...)
  {
    mexErrMsgTxt("ERROR: Exception caught!");
  }
  
}

void ssc_close() {
  client->stopReceiving();
  if(0 != client) {
    delete client;
  }
  
  client = 0;
}
