/*
 * mexSSClient.c - get data from the signalserver
 *
 * For information about the usage and compiling see mexSSClient.m
 *
 * The mexfile uses the cpp client of the signalserver to connect to the 
 * signalserver. Therefore it uses also the data structures of the 
 * signalserver to ensure the optimal compatibly. The configuration, 
 * data aquistion and closing of the connection are handeld in seperated methods.
 * Because you have to call mexSSClient again for each usecase, we save the 
 * SSClient in a static structure. You have to be careful to close and 
 * reinitialise it otherwise you will get a null pointer exception in the 
 * data acquisition part of the program.
 *
 * Author 
 *    Max Sagebaum 
 *    with help and code from Christian Breitwieser
 * 
 *  2010/03/17 - Max Sagebaum
 *               - file created   
 *  2010/04/07 - Max Sagebaum
 *               - added comments and version history
 *               - new return structure in getData
 *  2010/10/08 - Max Sagebaum
 *               - new error handling
 *               - we close the old connection now if we want to connect 
 *                 and a connection still exists.
 */

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
#include <boost/cast.hpp>

#include "ticpp/ticpp.h"

#include "datapacket/data_packet.h"
#include "definitions/constants.h"
#include "signalserver-client/ssclient.h"
#include "signalserver-client/ssconfig.h"

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

#define NUM_OUTPUTS_CONFIG   3
#define FS_BS_POS        0    //master sampling rate and blocksize
#define SIG_TYPES_POS    1    //signal types info (signal type (flag), signal type (string), blocksizes, nr of channels, fs)
#define CH_INFO_POS      2    //channel information
#define NR_SIG_TYPES_INFO  5  // number of fields in the sinal type info
#define NR_CH_INFO       2    // size of the channel info structure

#define NUM_OUTPUTS_GETDATA   2
#define INFO_STRUCT 0         // structure with general data
#define DATA_CELL 1           // cell matrix with the datablocks (signal name, signal data)
using boost::asio::ip::udp;
using namespace std;
using namespace tobiss;

#ifdef WIN32
using namespace boost;
#endif

static SSClient* client = 0;    // Static value for the client. We need to save it to obtain the data after the first configuration.
static std::map<uint32_t, Signal> *sig_types_map;   // The maps for the internal number of the signal type to the structure.


// ------------------------------------------------------------------------

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
  
  /** 
   * first check if we already have a connection
   */
  
  if(0 != client) {
    mexWarnMsgTxt ("mexSSClient: Warning connection already open. Closing the connection.");
    ssc_close();
  }

  /* Check for the number of arguments and check if the arguments have the 
   * correct format.
   */
  if(nlhs != NUM_OUTPUTS_CONFIG) {
    mexErrMsgTxt("mexSSClient: 4 output arguments required.");
  }

  if ( !mxIsChar(prhs[IP_POS]) ) {
    mexErrMsgTxt("mexSSClient: IP must be a string.");
  }
  if ( !mxIsNumeric(prhs[PORT_POS]) ) {
    mexErrMsgTxt("mexSSClient: Port must be a number.");
  }
  if ( !mxIsChar(prhs[PROTOCOL_POS]) ) {
    mexErrMsgTxt("mexSSClient: Protocol must be a string.");
  }

  if (mxGetM(prhs[IP_POS]) !=1 ) {
    mexErrMsgTxt("mexSSClient: IP must be a row vector.");
  }
  if (mxGetM(prhs[PORT_POS]) !=1 &&  mxGetN(prhs[PORT_POS]) !=1) {
    mexErrMsgTxt("mexSSClient: Port must be a single number.");
  }
  if (mxGetM(prhs[PROTOCOL_POS]) !=1 ) {
    mexErrMsgTxt("mexSSClient: Protocol must be a row vector.");
  }
  
  try
  {
    // read the input values
    string srv_addr(mxArrayToString(prhs[IP_POS]));
    string protocol(mxArrayToString(prhs[PROTOCOL_POS]));
    port = mxGetScalar(prhs[PORT_POS]);

    Constants cst;

    cout << "Using server " << srv_addr << ":" << port << endl;
    // Connect to the server
    client = new SSClient();
    sig_types_map = new std::map<uint32_t, Signal>();
    
    
    // connect to the server and get the config file
    client->connect(srv_addr, port);

    client->requestConfig();
    SSConfig config = client->config();
    SignalInfo::SignalMap& signals(config.signal_info.signals());
    
    SignalInfo::SignalMap::iterator iter(signals.begin());

    for( ; iter != signals.end(); iter++) {
      sig_types_map->insert( make_pair(cst.getSignalFlag(iter->first), iter->second)  );
    }

    std::map<uint32_t, Signal>::iterator it(sig_types_map->begin());

    vector<uint32_t> sig_types;
    vector<uint16_t> blocksizes;
    vector<uint32_t> fs_per_sig_type;
    vector<uint16_t> ch_per_sig_type;

    map<uint32_t, vector<string> >  naming;
    uint32_t nr_ch = 0;

    // get the infomation from the  config
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

    // create and populate the matlab structures
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
    string ex_str("mexSSClient:  ***** TICPP Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::invalid_argument& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Invalid argument -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::length_error& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Length error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::logic_error& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Logic error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::range_error& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Range error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::runtime_error& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Runtime error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(std::exception& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(boost::exception& e)
  {
    string ex_str("mexSSClient:  ***** Boost Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    mexErrMsgTxt(ex_str.c_str());
  }
  catch(...)
  {
    mexErrMsgTxt("mexSSClient: ERROR !!  --  Unknown exception caught!");
  }
    return;
}

void ssc_getData(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  
  /** 
   * first check if we have a connection
   */  
  if(0 == client) {
    mexErrMsgTxt("mexSSClient: No connection open. Open a connection first");
    return;
  }

  if(nlhs != NUM_OUTPUTS_GETDATA) {
    mexErrMsgTxt("mexSSClient:2 output arguments required.");
  }
                    
  try
  {
    uint16_t nr_values = 0;
    vector<double> v;
    DataPacket packet;
 
    if (!client->receiving())
    {
      mexErrMsgTxt("mexSSClient: Receiving failed!");
    }

     // Get the data. For every signal type we generate a matrix with the data.
     // This data matrix we will put into a cellmatrix. The cellmatrix contains
     // for every signal type one row where the first coloumn contains the name 
     // of the signal and the second coloumn contains the data for that signal
     // type.
     client->getDataPacket(packet);
     mxArray* cellArray = mxCreateCellMatrix(packet.getNrOfSignalTypes(),2);
     plhs[DATA_CELL] = cellArray;

     mxArray* infoStruct = mxCreateStructMatrix(1,1, 0, 0);
     plhs[INFO_STRUCT] = infoStruct;

     mxAddField(infoStruct,"sampleNr");
     mxSetField(infoStruct,0,"sampleNr",mxCreateDoubleScalar(packet.getSampleNr()));
     
     unsigned int cellIndex = 0;
     for(map<uint32_t, Signal >::iterator it = sig_types_map->begin(); it != sig_types_map->end();  it++)
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
  }
  catch(bad_numeric_cast& e)
  {
    string ex_str("mexSSClient:  ***** Boost Numeric Cast Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(ticpp::Exception& e)
  {
    string ex_str("mexSSClient:  ***** TICPP Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::invalid_argument& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Invalid argument -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::length_error& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Length error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::logic_error& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Logic error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::range_error& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Range error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::runtime_error& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception -- Runtime error -- caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(std::exception& e)
  {
    string ex_str("mexSSClient:  ***** STL Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(boost::exception& e)
  {
    string ex_str("mexSSClient:  ***** Boost Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    mexErrMsgTxt( ex_str.c_str());
  }
  catch(...)
  {
    mexErrMsgTxt("mexSSClient: ERROR: Exception caught!");
  }
  
}

void ssc_close() {  
  if(0 != client) {
    try {
     client->stopReceiving();
    } catch( ...) {
      /* empty */
    }
    delete client;
  }
  if(0 != sig_types_map) {
    delete sig_types_map;
  }
  
  client = 0;
  sig_types_map = 0;
}
