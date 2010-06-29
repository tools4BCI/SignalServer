
#define S_FUNCTION_LEVEL 2
#define S_FUNCTION_NAME  simulink_send_data

#include "simstruc.h"

// #define IS_PARAM_DOUBLE(pVal) (mxIsNumeric(pVal) && !mxIsLogical(pVal) &&\
!mxIsEmpty(pVal) && !mxIsSparse(pVal) && !mxIsComplex(pVal) && mxIsDouble(pVal))

#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <algorithm>

// #include <boost/asio.hpp>
// #include <boost/bind.hpp>
// #include <boost/thread.hpp>
// #include <boost/exception/all.hpp>
// #include <boost/numeric/conversion/cast.hpp>

#include "../defines.h"
// #include "../ticpp/ticpp.h"

#ifdef WINDOWS
#include <boost/cstdint.hpp>
#else
#include <stdint.h>
#endif

using namespace std;
using namespace tobiss;

// using boost::numeric_cast;
// using boost::numeric::bad_numeric_cast;
// using boost::numeric::positive_overflow;
// using boost::numeric::negative_overflow;

#define NUM_WORK_POINTERS       1
#define SIGNAL_TYPES_MAP_POSITION      0

#define NUM_PARAMS 4
#define FS_BS_POS       0   //master sampling rate and blocksize
#define SIG_TYPES_POS   1    //signal types info (signal type (flag), signal type (string), blocksizes, nr of channels, fs)
    #define NR_SIG_TYPES_INFO   5
#define CH_INFO_POS     2    //channel information
    #define NR_CH_INFO          2
#define SERVER_INFO_POS 3
    #define NR_SERVER_INFO      3   // protocol, ip, port


//---------------------------------------------------------------------------------------

void calcSizes(const mxArray* sig_types, vector<int>& width, vector< pair<int,int> >& dims)
{
  //   Signal Types Info:  (Flag, Type, BS, NrCh)
  //   [1]    'eeg'    [1]    [1]
  //   [4]    'eog'    [1]    [4]
  //   [8]    'ecg'    [2]    [1]

  uint16_t nr_sig_types = mxGetM(sig_types);
  vector<uint16_t> channels(nr_sig_types);
  vector<uint16_t> blocks(nr_sig_types);

  for(uint32_t n = 0; n < nr_sig_types; n++)
  {
    channels[n] = mxGetScalar(mxGetCell(sig_types, 3*nr_sig_types +n));
    blocks[n]   = mxGetScalar(mxGetCell(sig_types, 2*nr_sig_types +n));
  }

  for(unsigned int n = 0; n < channels.size(); n++)
    width.push_back(channels[n]*blocks[n]);

  for(unsigned int n = 0; n < width.size(); n++)
    dims.push_back( make_pair(channels[n], blocks[n] ) );

}

//---------------------------------------------------------------------------------------

static void mdlInitializeSizes(SimStruct *S)
{
  ssAllowSignalsWithMoreThan2D(S);

  ssSetNumSFcnParams(S,  NUM_PARAMS);
  if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
    return;

  if(mxIsEmpty(ssGetSFcnParam(S,FS_BS_POS)) || !mxIsNumeric( ssGetSFcnParam(S,FS_BS_POS))
    || (mxGetM(ssGetSFcnParam(S,FS_BS_POS)) > 1) || (mxGetN(ssGetSFcnParam(S,FS_BS_POS)) != 2) )
  {
    ssSetErrorStatus(S,"Parameter 1 must be two single integer!");
    return;
  }

  if(mxIsEmpty(ssGetSFcnParam(S,SIG_TYPES_POS)) || !mxIsCell( ssGetSFcnParam(S,SIG_TYPES_POS))
    || (mxGetN(ssGetSFcnParam(S,SIG_TYPES_POS)) != NR_SIG_TYPES_INFO) )
  {
    ssSetErrorStatus(S,"Parameter 2 not correct!");
    return;
  }

  if(mxIsEmpty(ssGetSFcnParam(S,CH_INFO_POS)) || !mxIsCell( ssGetSFcnParam(S,CH_INFO_POS))
    || (mxGetN(ssGetSFcnParam(S,CH_INFO_POS)) != NR_CH_INFO) )
  {
    ssSetErrorStatus(S,"Parameter 3 not correct!");
    return;
  }

  ssSetNumPWork(S, NUM_WORK_POINTERS);

  if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
    return;

  const mxArray* fs_bs = ssGetSFcnParam(S, FS_BS_POS);
  const mxArray* sig_types = ssGetSFcnParam(S, SIG_TYPES_POS);
  double fs_master = mxGetPr(fs_bs)[0];
  double bs_master = mxGetPr(fs_bs)[1];

  vector<int> width;
  vector< pair<int,int> > dims;
  calcSizes(sig_types, width, dims);

  uint16_t nr_sig_types = mxGetM(sig_types);
  vector<uint16_t> fs(nr_sig_types);
  uint16_t max_fs = 0;

  for(uint32_t n = 0; n < nr_sig_types; n++)
    fs[n] = mxGetScalar(mxGetCell(sig_types, 4*nr_sig_types +n));
  max_fs = *max_element(fs.begin(), fs.end());
  fs.push_back(max_fs);


  mexPrintf("TEST 1\n");

  if(!ssSetNumInputPorts(S, fs.size()))    // Events ... last port
    return;
  mexPrintf("TEST 2\n");
  for(unsigned int n = 0; n < fs.size(); n++)
  {
    ssSetInputPortFrameData(S, n, FRAME_INHERITED);

    if(!ssSetInputPortMatrixDimensions(S, n, DYNAMICALLY_SIZED, DYNAMICALLY_SIZED))
      return;

  }

mexPrintf("TEST 3\n");

  ssSetOptions(S, SS_OPTION_ALLOW_CONSTANT_PORT_SAMPLE_TIME);
  ssSetNumSampleTimes(S, PORT_BASED_SAMPLE_TIMES);
  mexPrintf("TEST 4\n");

  for(unsigned int n = 0; n < fs.size(); n++)
  {
    DECL_AND_INIT_DIMSINFO(di);

    di.numDims = 2;
    di.width   = DYNAMICALLY_SIZED;
//     dimensions[0] = dims[n].second;    //  rows
//     dimensions[1] = dims[n].first;     //  columns

    di.dims    = [DYNAMICALLY_SIZED DYNAMICALLY_SIZED];

    if(!ssSetInputPortDimensionInfo(S, n, &di))
      return;

    mexPrintf(" -- loop\n");
    ssSetInputPortSampleTime(S, n, INHERITED_SAMPLE_TIME);
    ssSetInputPortOffsetTime(S, n, 0);
  }
  mexPrintf("TEST 5\n");
}

//---------------------------------------------------------------------------------------

static void mdlInitializeSampleTimes(SimStruct *S)
{

}

//---------------------------------------------------------------------------------------

#define MDL_START
static void mdlStart(SimStruct *S)
{
  mexPrintf("TEST 6\n");
}

//---------------------------------------------------------------------------------------

static void mdlOutputs(SimStruct *S, int_T tid)
{

}

//---------------------------------------------------------------------------------------

static void mdlTerminate(SimStruct *S)
{

}

//---------------------------------------------------------------------------------------

  // Required S-function trailer
  #ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
  #include "simulink.c"      /* MEX-file interface mechanism */
  #else
  #include "cg_sfun.h"       /* Code generation registration function */
  #endif
