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

#include "mex.h"
#include "matrix.h"

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iostream>

#include <boost/exception/all.hpp>


#include "tia/constants.h"
#include "tia/tia_client.h"
#include "tia/ssconfig.h"

#include <boost/cstdint.hpp>

enum
{
  IP_POS = 0,
  PORT_POS,
  TIA_VERSION_POS,
  NUM_INPUTS	// must be last
};

enum
{
  MASTER_FS_POS = 0,
  MASTER_BLOCKSIZE_POS,
  SIG_TYPES_POS,
  CH_INFO_POS,
  NUM_OUTPUTS	// must be last
};

#define NR_SIG_TYPES_INFO   5
#define NR_CH_INFO   2

using boost::uint16_t;
using boost::uint32_t;

using namespace std;
using namespace tobiss;


//---------------------------------------------------------------------------------------

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
  uint32_t port;

  if(nrhs != NUM_INPUTS)  // IP, port, protocol
    mexErrMsgTxt("3 input arguments required.");
  if(nlhs != NUM_OUTPUTS)
    mexErrMsgTxt("3 output arguments required.");

  if ( !mxIsChar(prhs[IP_POS]) )
    mexErrMsgTxt("IP must be a string.");
  if ( !mxIsNumeric(prhs[PORT_POS]) )
    mexErrMsgTxt("Port must be a number.");

  if (mxGetM(prhs[IP_POS]) !=1 )
    mexErrMsgTxt("IP must be a row vector.");
  if (mxGetM(prhs[PORT_POS]) !=1 &&  mxGetN(prhs[PORT_POS]) !=1)
    mexErrMsgTxt("Port must be a single number.");

  if ( !mxIsChar(prhs[TIA_VERSION_POS]) )
    mexErrMsgTxt("IP must be a string.");
  if (mxGetM(prhs[TIA_VERSION_POS]) !=1 )
    mexErrMsgTxt("IP must be a row vector.");

  string tia_version( mxArrayToString(prhs[TIA_VERSION_POS]) );

  bool use_new_tia = true;

  if(tia_version == "0.1")
    use_new_tia = false;
  else if(tia_version != "0.2")
    mexErrMsgTxt("Unknown TiA version!");
  
  try
  {
    string srv_addr(mxArrayToString(prhs[IP_POS]));
    port = mxGetScalar(prhs[PORT_POS]);

    Constants cst;

    cout << "Using server " << srv_addr << ":" << port;
    cout << "  (TiA version: " << tia_version << ")"<< endl;
    TiAClient client;
    client.connect(srv_addr, port, use_new_tia);

    client.requestConfig();
    SSConfig config = client.config();
    tobiss::SignalInfo::SignalMap& signals(config.signal_info.signals());
    std::map<uint32_t, tobiss::Signal> sig_types_map;
    tobiss::SignalInfo::SignalMap::iterator iter(signals.begin());

    for( ; iter != signals.end(); iter++)
      sig_types_map.insert( make_pair(cst.getSignalFlag(iter->first), iter->second)  );

    std::map<uint32_t, tobiss::Signal>::iterator it(sig_types_map.begin());

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

    plhs[MASTER_FS_POS] =
      mxCreateDoubleScalar( config.signal_info.masterSamplingRate() );
    plhs[MASTER_BLOCKSIZE_POS] =
      mxCreateDoubleScalar( config.signal_info.masterBlockSize() );
    

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
