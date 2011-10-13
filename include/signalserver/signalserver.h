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

#ifndef SIGNALSERVER_H
#define SIGNALSERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>


#ifdef USE_TID_SERVER
  namespace TiD
  {
    class TiDServer;
  }
#endif

#ifdef USE_GDF_SAVER
  namespace gdf
  {
    class Writer;
  }
#endif

namespace tia
{
  class TiAServer;
  class DataPacket;
}

namespace tobiss
{

class XMLParser;
class HWAccess;


//-----------------------------------------------------------------------------

class SignalServer
{
  public:
    SignalServer(XMLParser& config_parser, bool use_new_tia);
    virtual ~SignalServer();

    void stop();
    void readPackets();

    static std::vector<std::string> getPossibleHardwareNames();

  private:
    void initGdf(); ///< Initialize writing acquired data into a .gdf file.

    HWAccess*                   hw_access_;
    tia::TiAServer*             tia_server_;

    boost::asio::io_service     tia_io_service_;
    boost::thread*              tia_io_service_thread_;

    boost::asio::io_service     hw_access_io_service_;
    boost::thread*              hw_access_io_service_thread_;


    XMLParser&                  config_parser_;

    bool                                stop_reading_;
    bool                                write_file_;
    boost::uint32_t                     master_blocksize_;
    boost::uint32_t                     master_samplingrate_;
    std::map<std::string,std::string>   subject_info_;
    std::map<std::string,std::string>   server_settings_;
    std::map<boost::uint32_t, std::vector<std::string> >    channels_per_sig_type_;
    std::vector<boost::uint32_t>                            sampling_rate_per_sig_type_;

    tia::DataPacket*                                        packet_;

    #ifdef USE_TID_SERVER
      TiD::TiDServer*             tid_server_;
      boost::asio::io_service     tid_io_service_;
      boost::thread*              tid_io_service_thread_;
    #endif

    #ifdef USE_GDF_SAVER
      gdf::Writer*                gdf_writer_;
    #endif
};

//-----------------------------------------------------------------------------

} // tobiss

#endif // SIGNALSERVER_H
