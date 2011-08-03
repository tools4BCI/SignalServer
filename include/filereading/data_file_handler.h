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
* @file data_file_handler.h
*
* @brief
*
**/

#ifndef DATAFILEHANDLER_H
#define DATAFILEHANDLER_H

#include <vector>
#include <map>

//#include <boost/asio.hpp>
//#include <boost/thread/condition.hpp>  // for mutex and cond. variables
//#include <boost/thread/shared_mutex.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/cstdint.hpp>


#include "tia/defines.h"
#include "tia/data_packet.h"

#include "filereading/file_reader.h"
#include "filereading/file_reader_factory.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class DataFileHandler
*
* @brief
*
*/
class DataFileHandler
{
  public:
    /**
    *
    */
    //DataFileHandler(boost::asio::io_service& io, std::map<std::string, std::string> config);

    /**
    * @brief Destructor
    */
    virtual ~DataFileHandler()    {   }


    DataPacket getDataPacket();

    /**
    * @brief Method to start data acquisition.
    */
    void run();

    /**
    * @brief Method to stop data acquisition.
    */
    void stop();

    void readSamples();

//-----------------------------------------------
  private:



//-----------------------------------------------

  private:

    //boost::asio::io_service&                  io_service_;
    std::map<std::string, std::string>        config_;

    //FileReader*                               current_reader_;

    //FileReaderFactory                         reader_factory_;



    //boost::asio::deadline_timer* t_;   ///< timer object for accurate timing
    //double step_;        ///< needed for sine generation
    //double cycle_dur_;   ///< needed for sine generation
    //boost::posix_time::microseconds td_;   ///< time period for the timer

    //boost::mutex sync_mut_;  ///< mutex neede for synchronisation
    //boost::condition_variable_any cond_;   ///< condition variable to wake up getSyncData()

};

} // Namespace tobiss

#endif // DATAFILEHANDLER_H

//-----------------------------------------------------------------------------
