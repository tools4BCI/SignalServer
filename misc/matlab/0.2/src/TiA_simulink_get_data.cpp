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

#define S_FUNCTION_LEVEL 2
#define S_FUNCTION_NAME  TiA_simulink_get_data

#include "simstruc.h"

#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <iostream>


#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include "tia/defines.h"
#include "tia/tia_client.h"
#include "tia/data_packet_interface.h"

#include <boost/cstdint.hpp>

using namespace std;
using namespace tia;

using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
using boost::numeric_cast;
using boost::lexical_cast;
using boost::numeric::bad_numeric_cast;
using boost::numeric::positive_overflow;
using boost::numeric::negative_overflow;

enum
{
  SIGNAL_TYPES_MAP_POSITION = 0,
  TiAClient_POSITION,
  START_TIME_POSITION,
  NUM_WORK_POINTERS 	// must be last
};

enum
{
  MASTER_FS_POS = 0,
  MASTER_BLOCKSIZE_POS,
  SIG_TYPES_POS,
  IP_POS,
  PORT_POS,
  PROTOCOL_POS,
  TIA_VERSION_POS,
  NUM_PARAMS	// must be last
};

#define NR_SIG_TYPES_INFO   4

#define MAX_NR_OF_EVENTS_AT_ONCE  128

//---------------------------------------------------------------------------------------

double round(double d)
{
  return floor(d + 0.5);
}

//---------------------------------------------------------------------------------------

void calcSizes(const mxArray* sig_types, vector<int>& width, vector< pair<uint16_t,uint16_t> >& dims)
{
  //   Signal Types Info:  (Flag, BS, NrCh, fs)
  //   [1]      [1]    [1]    [100]
  //   [4]      [1]    [4]    [10]
  //   [8]      [2]    [1]    [50]

  uint16_t nr_sig_types = mxGetM(sig_types);
  vector<uint16_t> channels(nr_sig_types);
  vector<uint16_t> blocks(nr_sig_types);

  
  for(uint32_t n = 0; n < nr_sig_types; n++)
  {
    blocks[n]   = mxGetPr(sig_types)[   nr_sig_types +n ];
    channels[n] = mxGetPr(sig_types)[ 2*nr_sig_types +n ];
  }

  for(unsigned int n = 0; n < channels.size(); n++)
    width.push_back(channels[n]*blocks[n]);

  for(unsigned int n = 0; n < width.size(); n++)
    dims.push_back( make_pair(channels[n], blocks[n] ) ); 
}


//---------------------------------------------------------------------------------------

void setStaticOutputPorts(SimStruct *S, double fs_master, double bs_master)
{
  int dimensions[2];

  // set events output port
  ssSetOutputPortFrameData(S, 0, FRAME_NO);
  ssSetOutputPortMatrixDimensions(S, 0, DYNAMICALLY_SIZED, DYNAMICALLY_SIZED);
  DECL_AND_INIT_DIMSINFO(di);
  di.width   = MAX_NR_OF_EVENTS_AT_ONCE;
  di.numDims = 2;
  dimensions[0] = 1;    //  rows
  dimensions[1] = MAX_NR_OF_EVENTS_AT_ONCE;     //  columns
  di.dims    = dimensions;
  if(!ssSetOutputPortDimensionInfo(S, 0, &di))
    return;
  ssSetOutputPortSampleTime(S, 0, bs_master/fs_master);
  ssSetOutputPortOffsetTime(S, 0, 0);

  // set sample nr, timestamp and sys/simtime port


  
  for(unsigned int n = 1; n <= 3; n++ )
  {
    di.width   = 1;
    di.numDims = 2;
    dimensions[0] = 1;    //  rows
    dimensions[1] = 1;     //  columns
    di.dims    = dimensions;
    ssSetOutputPortFrameData(S, n, FRAME_NO);
    ssSetOutputPortMatrixDimensions(S, n, DYNAMICALLY_SIZED, DYNAMICALLY_SIZED);

    if(!ssSetOutputPortDimensionInfo(S, n, &di))
      return;
    ssSetOutputPortSampleTime(S, n, bs_master/fs_master);
    ssSetOutputPortOffsetTime(S, n, 0);
  }

}

//---------------------------------------------------------------------------------------
// master_fs, master_bs, sig_info, ip, port, proto, tia_version
static void mdlInitializeSizes(SimStruct *S)
{
  ssAllowSignalsWithMoreThan2D(S);

  ssSetNumSFcnParams(S,  NUM_PARAMS);
  if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
    return;

  if(mxIsEmpty(ssGetSFcnParam(S,MASTER_FS_POS)) || !mxIsNumeric( ssGetSFcnParam(S,MASTER_FS_POS))
    || (mxGetM(ssGetSFcnParam(S,MASTER_FS_POS)) > 1) || (mxGetN(ssGetSFcnParam(S,MASTER_FS_POS)) > 1) )
  {
    ssSetErrorStatus(S,"Parameter 1 must be an integer representing the masters sampling rate!");
    return;
  }

  if(mxIsEmpty(ssGetSFcnParam(S,MASTER_BLOCKSIZE_POS)) || !mxIsNumeric( ssGetSFcnParam(S,MASTER_BLOCKSIZE_POS))
    || (mxGetM(ssGetSFcnParam(S,MASTER_BLOCKSIZE_POS)) > 1) || (mxGetN(ssGetSFcnParam(S,MASTER_BLOCKSIZE_POS)) > 1) )
  {
    ssSetErrorStatus(S,"Parameter 2 must be an integer representing the masters blocksize!");
    return;
  }

  if(mxIsEmpty(ssGetSFcnParam(S,SIG_TYPES_POS)) || !mxIsNumeric( ssGetSFcnParam(S,SIG_TYPES_POS))
    || (mxGetN(ssGetSFcnParam(S,SIG_TYPES_POS)) != NR_SIG_TYPES_INFO) )
  {
    ssSetErrorStatus(S,"Parameter 3 not correct!");
    return;
  }

 if(mxIsEmpty(ssGetSFcnParam(S,IP_POS)) || !mxIsChar( ssGetSFcnParam(S,IP_POS)) )
 {
   ssSetErrorStatus(S,"Parameter 4 not correct!");
   return;
 }
  if(mxIsEmpty(ssGetSFcnParam(S,PORT_POS)) || !mxIsNumeric( ssGetSFcnParam(S,PORT_POS)) )
  {
    ssSetErrorStatus(S,"Parameter 5 not correct!");
    return;
  }
 if(mxIsEmpty(ssGetSFcnParam(S,PROTOCOL_POS)) || !mxIsChar( ssGetSFcnParam(S,PROTOCOL_POS)) )
 {
   ssSetErrorStatus(S,"Parameter 6 not correct!");
   return;
 }
 if(mxIsEmpty(ssGetSFcnParam(S,TIA_VERSION_POS)) || !mxIsChar( ssGetSFcnParam(S,PROTOCOL_POS)) )
 {
   ssSetErrorStatus(S,"Parameter 7 not correct!");
   return;
 }

  ssSetNumPWork(S, NUM_WORK_POINTERS);

  if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
    return;

  const mxArray* sig_types = ssGetSFcnParam(S, SIG_TYPES_POS);
  double fs_master = mxGetScalar( ssGetSFcnParam(S, MASTER_FS_POS));
  double bs_master = mxGetScalar( ssGetSFcnParam(S, MASTER_BLOCKSIZE_POS));

  vector<int> width;
  vector< pair<uint16_t,uint16_t> > dims;
  calcSizes(sig_types, width, dims);

  uint16_t nr_sig_types = mxGetM(sig_types);
  vector<uint16_t> fs(nr_sig_types);

  for(uint32_t n = 0; n < nr_sig_types; n++)
    fs[n] = mxGetPr( sig_types)[3*nr_sig_types +n];

  //-----------------------------
  // set output ports sizes

  if (!ssSetNumInputPorts(S, 0))
    return;
  
  if (!ssSetNumOutputPorts(S, width.size() + 4))    // events, samplenr, timestamp and system/model time difference ... last 4 ports
    return;

  ssSetOptions(S, SS_OPTION_DISALLOW_CONSTANT_SAMPLE_TIME );
  ssSetNumSampleTimes(S, PORT_BASED_SAMPLE_TIMES);
  
  setStaticOutputPorts(S, fs_master, bs_master);

  for(unsigned int n = 0; n < width.size(); n++)
  {
    if(dims[n].second > 1 )
      ssSetOutputPortFrameData(S, n+4, FRAME_YES);
    else
      ssSetOutputPortFrameData(S, n+4, FRAME_NO);

    ssSetOutputPortMatrixDimensions(S, n+4, DYNAMICALLY_SIZED, DYNAMICALLY_SIZED);
  }

  int dimensions[2];
  for(unsigned int n = 0; n < width.size(); n++)
  {

    DECL_AND_INIT_DIMSINFO(di);

    di.width   = width[n];
    di.numDims = 2;
    dimensions[0] = dims[n].second;    //  rows
    dimensions[1] = dims[n].first;     //  columns

    di.dims    = dimensions;

    if(!ssSetOutputPortDimensionInfo(S, n+4, &di))
      return;

    if(fs[n])
      ssSetOutputPortSampleTime(S, n+4, numeric_cast<double>(dims[n].second) / numeric_cast<double>(fs[n]) );
    else
      ssSetOutputPortSampleTime(S, n+4, bs_master/fs_master );

    ssSetOutputPortOffsetTime(S, n+4, 0);

    ///     FIXME: If blocked transmission is used, data is delayed by "n"  samples --
    ///        now solved by a delay-block, maybe possible through ssSetOutputPortOffsetTime
    //     ssSetOutputPortOffsetTime(S, n, -dims[n].second/fs[n]);
  }
}

//---------------------------------------------------------------------------------------

static void mdlInitializeSampleTimes(SimStruct *S)
{
//   //   ssSetSampleTime(S, 0, mxGetPr(ssGetSFcnParam(S,FS_BS_POS))[1] );  //  ... has to be the same as the number of blocks of the master (rows)
//   //   ssSetSampleTime(S, 0, ssGetOutputPortDimensions(S, 0)[0] );   // temporary ... will be given as parameter from config while initialization
//   //   ssSetSampleTime(S, 0, 1/256 );
//   //   ssSetOffsetTime(S, 0, 0.0);
//   //   ssSetModelReferenceSampleTimeDefaultInheritance(S);
}

//---------------------------------------------------------------------------------------

#define MDL_START
static void mdlStart(SimStruct *S)
{
  try
  {
    string ip = mxArrayToString( ssGetSFcnParam(S, IP_POS) );
    string protocol = mxArrayToString( ssGetSFcnParam(S, PROTOCOL_POS) );
    string tia_version( mxArrayToString( ssGetSFcnParam(S, TIA_VERSION_POS)) );
    uint32_t ctrl_port = mxGetScalar( ssGetSFcnParam(S, PORT_POS) );

    bool use_new_tia = true;

    if(tia_version == "0.1")
      use_new_tia = false;
    else if(tia_version != "0.2")
      ssSetErrorStatus(S, "Unknown TiA version!");

    map<uint32_t, pair<uint16_t, uint16_t> >* sig_info =
        new map<uint32_t, pair<uint16_t, uint16_t> >;  // (flag, (channels,blocks) )

    const mxArray* sig_types = ssGetSFcnParam(S, SIG_TYPES_POS);
        //   Signal Types Info:  (Flag, BS, NrCh, FS)
        //   [1]       [1]    [1]   [512]
        //   [4]       [1]    [4]   [512]
        //   [8]       [2]    [1]   [128]

    uint16_t nr_sig_types = mxGetM(sig_types);
    vector<uint16_t> channels(nr_sig_types);
    vector<uint16_t> blocks(nr_sig_types);

    for(uint32_t n = 0; n < nr_sig_types; n++)
    {
      uint32_t flag = numeric_cast<uint32_t>( mxGetPr( sig_types)[n] );
      uint16_t channels = mxGetPr(sig_types)[ 2*nr_sig_types +n ] ;
      uint16_t blocks   = mxGetPr(sig_types)[   nr_sig_types +n ] ;
      sig_info->insert( make_pair( flag, make_pair(channels, blocks) ) );
    }

    ssGetPWork(S)[SIGNAL_TYPES_MAP_POSITION] =  sig_info;

    TiAClient* client = new TiAClient(use_new_tia);
    ssGetPWork(S)[TiAClient_POSITION] = client;
    client->connect(ip, ctrl_port);

    boost::posix_time::ptime* start_time =
          new boost::posix_time::ptime( boost::posix_time::microsec_clock::local_time() );
    ssGetPWork(S)[START_TIME_POSITION] = start_time;

    if((protocol == "udp") || (protocol == "tcp") )
    {
      bool udp =  (protocol == "udp");
      client->startReceiving(udp);
    }
    else
      ssSetErrorStatus(S, "Protocol neiter UDP or TCP!");

  }
  catch(bad_numeric_cast& e)
  {
    string ex_str(" ***** Boost Numeric Cast Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    ssSetErrorStatus(S, ex_str.c_str());
  }
  catch(std::exception& e)
  {
    string ex_str(" ***** STL Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    ssSetErrorStatus(S, ex_str.c_str());
  }
  catch(boost::exception& e)
  {
    string ex_str(" ***** Boost Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    ssSetErrorStatus(S, ex_str.c_str());
  }
  catch(...)
  {
    ssSetErrorStatus(S,"ERROR: Exception caught!");
  }
}

//---------------------------------------------------------------------------------------

static void mdlOutputs(SimStruct *S, int_T tid)
{  
  TiAClient* client = static_cast<TiAClient* >(ssGetPWork(S)[TiAClient_POSITION]);

  map<uint32_t, pair<uint16_t, uint16_t> >* sig_info =
  static_cast<map<uint32_t, pair<uint16_t, uint16_t> >* >(ssGetPWork(S)[SIGNAL_TYPES_MAP_POSITION]);
    // (flag, (channels,blocks) )

  boost::posix_time::ptime* start_time =
      static_cast<boost::posix_time::ptime*>(ssGetPWork(S)[START_TIME_POSITION]);

  try
  {
    uint16_t nr_values = 0;
    vector<double> v;
    DataPacket* packet = client->getEmptyDataPacket();

    if (!client->receiving())
    {
      ssSetErrorStatus(S, "Receiving failed!");
    }

    client->getDataPacket(*packet);
    unsigned int port = 0;
    real_T *y = 0;

    //     get possible event
    y = ssGetOutputPortRealSignal(S, port);
    try
    {
      //TODO -- implement TiD stuff
      y[0] = 0;
    }
    catch(std::invalid_argument& e)
    {
      y[0] = 0;
    }

    //     get sample nr
    y = ssGetOutputPortRealSignal(S, ++port);
    y[0] = packet->getPacketID();

    //     get timestamp
    y = ssGetOutputPortRealSignal(S, ++port);
    boost::uint64_t timestamp = packet->getTimestamp();

    y[0] = *(reinterpret_cast<real_T*>(&timestamp));

    //     calc difference of system time and the master-port simulation time
    y = ssGetOutputPortRealSignal(S, ++port);
    
    boost::posix_time::time_duration sim_time =  boost::posix_time::time_duration(0, 0,
                                                 numeric_cast<uint64_t>(  ssGetT(S) ) ,
                                                    numeric_cast<uint64_t>( (ssGetT(S) -
                                                    numeric_cast<uint64_t>( ssGetT(S) ) )*1000000) );
    
    boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time()
                                            - *start_time - sim_time;

    y[0] = lexical_cast<double>( to_iso_string(diff) );


    for(map<uint32_t, pair<uint16_t, uint16_t> >::iterator it = sig_info->begin();
        it != sig_info->end();  it++)
    {
      y = ssGetOutputPortRealSignal(S, ++port);

      try{
        v = packet->getSingleDataBlock(it->first);
        nr_values = packet->getNrOfSamples(it->first);

        for(unsigned int n = 0; n < nr_values; n++)
          y[n] = v[n];

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
    string ex_str(" ***** Boost Numeric Cast Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    ssSetErrorStatus(S, ex_str.c_str());
  }
  catch(std::exception& e)
  {
    string ex_str(" ***** STL Exception caught! *****\n  -->");
    ex_str += e.what();
    ex_str += '\n';
    ssSetErrorStatus(S, ex_str.c_str());
  }
  catch(boost::exception& e)
  {
    string ex_str(" ***** Boost Exception caught! *****\n  -->");
    ex_str += boost::diagnostic_information(e);
    ex_str += '\n';
    ssSetErrorStatus(S, ex_str.c_str());
  }
  catch(...)
  {
    ssSetErrorStatus(S,"ERROR: Exception caught!");
  }
}

//---------------------------------------------------------------------------------------

static void mdlTerminate(SimStruct *S)
{
  try
  {
    map<uint32_t, pair<uint16_t, uint16_t> >* sig_info =
      static_cast<map<uint32_t, pair<uint16_t, uint16_t> >* >(ssGetPWork(S)[SIGNAL_TYPES_MAP_POSITION]);
    if(sig_info)
      delete(sig_info);

    boost::posix_time::ptime* start_time =
      static_cast<boost::posix_time::ptime*>(ssGetPWork(S)[START_TIME_POSITION]);
    if(start_time)
      delete(start_time);

    TiAClient* client = static_cast<TiAClient* >(ssGetPWork(S)[TiAClient_POSITION]);
    if(client)
    {
      client->stopReceiving();

      if (client->receiving())
      {
        mexPrintf("Cannot Stop Receiving! \n");
      }

      delete(client);
    }
  }
  catch(...)
  {
    ssSetErrorStatus(S,"ERROR: Exception caught!");
  }
}

//---------------------------------------------------------------------------------------

  // Required S-function trailer
  #ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
  #include "simulink.c"      /* MEX-file interface mechanism */
  #else
  #include "cg_sfun.h"       /* Code generation registration function */
  #endif



    // former code:

    
    // calculate a timeformat readable for matlab  -- used at timestamp port

    // string str = to_iso_string(timestamp);
    // int pos = str.find("T");
    // string str2 = str.substr (pos+1);
    //
    // double hour = lexical_cast<double>(str2.substr (0,2));
    // double min  = lexical_cast<double>(str2.substr (2,2));
    // double sec  = lexical_cast<double>(str2.substr (4));
    //
    // y[0] = (3600*hour) + (60*min) + sec;
