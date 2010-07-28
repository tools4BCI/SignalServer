
/**
* @file gdf_file_reader.h
*
* @brief
*
**/

#ifndef GDFFILEREADER_H
#define GDFFILEREADER_H

#include <vector>
#include <map>

#include <boost/cstdint.hpp>

#include "definitions/defines.h"
#include "datapacket/data_packet.h"

namespace tobiss
{
  //-----------------------------------------------------------------------------
  /**
  * @class GdfFileReader
  *
  * @brief
  *
  */
  class GdfFileReader:FileReader
  {
    public:
      /**
      *
      */
      GdfFileReader();

      /**
      * @brief Destructor
      */
      virtual ~GdfFileReader()    {   }


      DataPacket getDataPacket()
      {
        DataPacket packet;
        return(packet);
      }

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

#endif // GDFFILEREADER_H

//-----------------------------------------------------------------------------
