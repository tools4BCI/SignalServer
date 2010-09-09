
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
#include "ticpp/ticpp.h"

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

      /**
      * @brief Checks, if the given std::string represents supported hardware.
      * @param[in] s std::string to be checked.
      * @return Unique identifier representing the given std::string.
      * @throw ticpp::Exception thrown if std::string representing hardware not found!
      *
      */
      int isSupportedHardware(const std::string& s);

      /**
      * @brief Checks, if the given std::string represents the Sine Generator.
      * @param[in] s std::string to be checked.
      * @return Bool
      *
      */
      bool isSineGen(const std::string& s);

      /**
      * @brief Checks, if the given std::string represents the g.tec g.USBamp.
      * @param[in] s std::string to be checked.
      * @return Bool
      *
      */
      bool isUSBamp(const std::string& s);

      /**
      * @brief Checks, if the given std::string represents the g.tec g.Mobilab+.
      * @param[in] s std::string to be checked.
      * @return Bool
      *
      */
      bool isMobilab(const std::string& s);

      /**
      * @brief Checks, if the given std::string represents a Joystick device.
      * @param[in] s std::string to be checked.
      * @return Bool
      *
      */
      bool isJoystick(const std::string& s);

      /**
      * @brief Maps given std::strings "on" or "off" to boolean values 0 or 1.
      * @param[in] s std::string to be checked.
      * @return Bool
      * @throw ticpp::Exception thrown if std::string neither on or off (or 0/1)!
      *
      */
      bool equalsOnOrOff(const std::string& s);

      /**
      * @brief Maps given std::strings "yes" or "no" to boolean values 0 or 1.
      * @param[in] s std::string to be checked.
      * @return Bool
      * @throw ticpp::Exception thrown if std::string neither yes or no (or 0/1)!
      *
      */
      bool equalsYesOrNo(const std::string& s);

      /**
      * @brief Checks, if the given std::string equals "master".
      * @param[in] s std::string to be checked.
      * @return Bool
      */
      bool equalsMaster(const std::string& s);

      /**
      * @brief Checks, if the given std::string equals "slave".
      * @param[in] s std::string to be checked.
      * @return Bool
      */
      bool equalsSlave(const std::string& s);

      /**
      * @brief Checks, if the given std::string equals "aperiodic".
      * @param[in] s std::string to be checked.
      * @return Bool
      */
      bool equalsAperiodic(const std::string& s);

      /**
      * @brief Maps a given std::string to the specific code of this filter at the the g.USBamp.
      * @param[in] s std::string to be checked.
      * @return FilterID
      * @throw ticpp::Exception thrown if filter name not found!
      *
      */
      int getUSBampFilterType(const std::string& s);

      /**
      * @brief Maps a given std::string to the specific OP_MODE of the g.USBamp.
      * @param[in] s std::string to be checked.
      * @return OP_MODE std::string
      * @throw ticpp::Exception thrown if OP_MODE name not found!
      *
      */
      std::string getUSBampOpMode(const std::string& s);

      /**
      * @brief Maps a given std::string to the specific g.USBamp channels group (naming on the front of the g.USBamp).
      * @param[in] s std::string to be checked.
      * @return block_id
      * @throw ticpp::Exception thrown if channel group naming not found!
      *
      */
      int getUSBampBlockNr(const std::string& s);

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

      static const std::string hardware;
        static const std::string hardware_name;
        static const std::string hardware_version;
        static const std::string hardware_serial;

        static const std::string hw_mode;
        static const std::string hw_ds;   ///< xml-tag hardware: device_settings
          static const std::string hw_fs;   ///< xml-tag hardware: sampling_rate

          static const std::string hw_channels;   ///< xml-tag hardware: measurement_channels
          static const std::string hw_ch_nr;   ///< xml-tag hardware: nr
          static const std::string hw_ch_names;   ///< xml-tag hardware: names
          static const std::string hw_ch_type;   ///< xml-tag hardware: channel type

          static const std::string hw_buffer;   ///< xml-tag hardware: blocksize

          static const std::string hw_fil;   ///< xml-tag hardware: filter
          static const std::string hw_fil_type;   ///< xml-tag hardware: filter type
          static const std::string hw_fil_order;   ///< xml-tag hardware: filter order
          static const std::string hw_fil_low;   ///< xml-tag hardware: filter lower cutoff freq.
          static const std::string hw_fil_high;   ///< xml-tag hardware: filter upper cutoff freq.

          static const std::string hw_notch;   ///< xml-tag hardware: notch_filter
          static const std::string hw_notch_center;    ///< xml-tag hardware: notch center freq.

        //USBamp specific start
          static const std::string hw_opmode;       ///< USBamp specific
          static const std::string hw_sc;            ///< USBamp specific
          static const std::string hw_trigger_line;            ///< USBamp specific
          static const std::string hw_usbampmaster;   ///< USBamp specific
          static const std::string hw_comgnd;        ///< USBamp specific
            static const std::string hw_gnd;          ///< USBamp specific
            static const std::string hw_gnd_block;   ///< USBamp specific
            static const std::string hw_gnd_value;   ///< USBamp specific

          static const std::string hw_comref;   ///< USBamp specific
            static const std::string hw_cr;       ///< USBamp specific
            static const std::string hw_cr_block;   ///< USBamp specific
            static const std::string hw_cr_value;   ///< USBamp specific
        //USBamp specific end

        static const std::string hw_cs;   ///< xml-tag hardware -- channel_settings
          static const std::string hw_sel;   ///< xml-tag hardware: selection
          static const std::string hw_cs_ch;   ///< xml-tag hardware: ch
          static const std::string hw_cs_nr;   ///< xml-tag hardware: nr
          static const std::string hw_cs_name;   ///< xml-tag hardware: name
          //naming of filter and others equal to global_settings

        //USBamp specific start
          static const std::string hw_bip;        ///< USBamp specific
          static const std::string hw_bip_with;   ///< USBamp specific
          static const std::string hw_drl;        ///< USBamp specific
          static const std::string hw_drl_value;   ///< USBamp specific
        //USBamp specific end

          //g.Mobilab specific start
          static const std::string hw_mobilab_serial_port;
          static const std::string hw_mobilab_type;
          static const std::string hw_mobilab_eeg;
          static const std::string hw_mobilab_multi;


    private:
      /**
      * @brief Mapping std::strings, representing hardware, and identifiers together.
      *
      */
      std::map<std::string, unsigned int> supported_hardware;

      /**
      * @brief Mapping std::strings, representing signaltypes, and identifiers together.
      *
      */
      std::map<std::string, boost::uint32_t> signaltypes;

      /**
      * @brief Mapping std::strings, representing g.USBamp filter types, and identifiers together.
      *
      */
      std::map<std::string, unsigned int> usbamp_filterTypes;

      /**
      * @brief Mapping std::strings, representing g.USBamp operation modes, and identifiers together.
      *
      */
      std::map<std::string, std::string> usbamp_opModes;

      /**
      * @brief Mapping std::strings, representing g.USBamp block namings, and identifiers together.
      *
      */
      std::map<std::string, unsigned int> usbamp_blockNames;

  };

} // Namespace tobiss


#endif // CONSTANTS_H

//-----------------------------------------------------------------------------
