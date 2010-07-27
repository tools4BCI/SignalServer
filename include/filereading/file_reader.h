
/**
* @file file_reader.h
*
* @brief
*
**/

#ifndef FILEREADER_H
#define FILEREADER_H

#include <vector>
#include <map>

#include <boost/cstdint.hpp>

#include "definitions/defines.h"
#include "datapacket/data_packet.h"

namespace tobiss
{
//-----------------------------------------------------------------------------
/**
* @class FileReader
*
* @brief
*
*/
class FileReader
{
  public:
    /**
    *
    */
    FileReader();

    /**
    * @brief Destructor
    */
    virtual ~FileReader()    {   }


    virtual DataPacket getDataPacket() = 0;

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


};

} // Namespace tobiss

#endif // FILEREADER_H

//-----------------------------------------------------------------------------
