/*
    This file is part of TOBI Interface A (TiA).

    TOBI Interface A (TiA) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface A (TiA) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface A (TiA).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

/**
* @file constants.h
*
* @brief Class constants manages all constants needed for the signalserver (especially xml tags)
*
* Constants is written to use the same naming in every file, connected to the SignalServer project.
* Definition of new naming in xml file or similar has to be done here!
* For complete comprehension also consider defines.h.
*
**/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <map>

#include <boost/cstdint.hpp>

#include "defines.h"

namespace tobiss
{
  /**
  * @class Constants
  *
  * @brief Class constants manages all constants needed for the signalserver (especially xml tags)
  *
  * This class is built to avoid hardcoded values or strings directly in the code, especially
  * needed for parsing xml strings.
  * All members are defined as public static const.
  * This class also manages hardware naming and checks predefined strings on correctnes
  * (e.g. master, slave, ...)
  * It also links defined SignalType values to their respective name (e.g. 0x01 -> eeg)
  *  --> see also defines.h
  *
  **/
  class Constants
  {

    public:
      /**
      * @brief Constructor
      *
      * Initialization of all private members.
      * Definition of new hardware naming or similar has to be done here!
      * Inserting data into private member variables (e.g. mapping std::string "eeg" and
      * SignalType definition for EEG together).
      */
      Constants();

      /**
      * @brief Default destructor
      *
      */
      virtual ~Constants()  { }

//      /**
//      * @brief Maps given strings "on" or "off" to boolean values 0 or 1.
//      * @param[in] s String to be checked.
//      * @return Bool
//      * @throw ticpp::Exception thrown if std::string neither on or off (or 0/1)!
//      *
//      */
//      bool equalsOnOrOff(const std::string& s);
//
//      /**
//      * @brief Maps given std::strings "yes" or "no" to boolean values 0 or 1.
//      * @param[in] s std::string to be checked.
//      * @return Bool
//      * @throw ticpp::Exception thrown if std::string neither yes or no (or 0/1)!
//      *
//      */
//      bool equalsYesOrNo(const std::string& s);
//
//      /**
//      * @brief Checks, if the given std::string equals "master".
//      * @param[in] s std::string to be checked.
//      * @return Bool
//      */
//      bool equalsMaster(const std::string& s);
//
//      /**
//      * @brief Checks, if the given std::string equals "slave".
//      * @param[in] s std::string to be checked.
//      * @return Bool
//      */
//      bool equalsSlave(const std::string& s);
//
//      /**
//      * @brief Checks, if the given std::string equals "aperiodic".
//      * @param[in] s std::string to be checked.
//      * @return Bool
//      */
//      bool equalsAperiodic(const std::string& s);

//      /**
//      * @brief Maps a given std::string to the specific code of this filter at the the g.USBamp.
//      * @param[in] s std::string to be checked.
//      * @return FilterID
//      * @throw ticpp::Exception thrown if filter name not found!
//      *
//      */
//      int getUSBampFilterType(const std::string& s);
//
//      /**
//      * @brief Maps a given std::string to the specific OP_MODE of the g.USBamp.
//      * @param[in] s std::string to be checked.
//      * @return OP_MODE std::string
//      * @throw ticpp::Exception thrown if OP_MODE name not found!
//      *
//      */
//      std::string getUSBampOpMode(const std::string& s);
//
//      /**
//      * @brief Maps a given std::string to the specific g.USBamp channels group (naming on the front of the g.USBamp).
//      * @param[in] s std::string to be checked.
//      * @return block_id
//      * @throw ticpp::Exception thrown if channel group naming not found!
//      *
//      */
//      int getUSBampBlockNr(const std::string& s);

      /**
      * @brief Maps a given std::string to the respective SignalType flag.
      * @param[in] s std::string to be checked.
      * @return flag
      * @throw ticpp::Exception thrown if std::string not representing any valid signaltype!
      *
      */
      boost::uint32_t getSignalFlag(const std::string& s);

      /**
      * @brief Returns the name to a given SignalType flaf.
      * @param[in] s Flag to be checked.
      * @return std::string
      * @throw ticpp::Exception thrown if flag does not represent any valid signaltype!
      *
      */
      std::string getSignalName(const boost::uint32_t& flag);

  //-----------------------------------------------

    public:
      //xml tags
      static const std::string tobi;

      static const std::string subject;
        static const std::string s_id;
        static const std::string s_first_name;
        static const std::string s_surname;
        static const std::string s_sex;
        static const std::string s_birthday;

      static const std::string ss;   ///< xml-tag serversettings
        static const std::string ss_ctl_port;
        static const std::string ss_udp_bc_addr;
        static const std::string ss_udp_port;

      static const std::string ss_store_data;
      static const std::string ss_filename;
      static const std::string ss_filetype;
      static const std::string ss_filepath;
      static const std::string ss_filepath_default;
      static const std::string ss_file_overwrite;
      static const std::string ss_file_overwrite_default;

      static const std::string file_reader;
        // filepath, name and type from store_data
      static const std::string fr_speedup;
      static const std::string fr_stop;

        //Mouse specific start
          static const std::string hw_vid;
          static const std::string hw_pid;
          static const std::string usb_port;
        //Mouse specific end

    private:
      /**
      * @brief Mapping std::strings, representing signaltypes, and identifiers together.
      *
      */
      std::map<std::string, boost::uint32_t> signaltypes;
  };

} // Namespace tobiss


#endif // CONSTANTS_H

//-----------------------------------------------------------------------------
