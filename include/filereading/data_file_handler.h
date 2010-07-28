
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

#include <boost/asio.hpp>
#include <boost/thread/condition.hpp>  // for mutex and cond. variables
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/cstdint.hpp>


#include "definitions/defines.h"
#include "datapacket/data_packet.h"

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
    DataFileHandler(boost::asio::io_service& io, std::map<std::string, std::string> config);

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

    boost::asio::io_service&                  io_service_;
    std::map<std::string, std::string>        config_;

    FileReader*                               current_reader_;

    FileReaderFactory                         reader_factory_;



    boost::asio::deadline_timer* t_;   ///< timer object for accurate timing
    double step_;        ///< needed for sine generation
    double cycle_dur_;   ///< needed for sine generation
    boost::posix_time::microseconds td_;   ///< time period for the timer

    boost::mutex sync_mut_;  ///< mutex neede for synchronisation
    boost::condition_variable_any cond_;   ///< condition variable to wake up getSyncData()

};

} // Namespace tobiss

#endif // DATAFILEHANDLER_H

//-----------------------------------------------------------------------------
