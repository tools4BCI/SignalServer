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
* @file hw_access.h
*
* @brief hw_access manages all objects representing different data acquisition devices.
*
* Devices are seperated into master and slave devices, whereby only one master is allowed
* at once, all other devices have to be slaves.
* The master device sets the pace DataPackets can be acquired as it is driven in a blocking way.
* Data from all slaves is gathered with the masters speed, whereby data stored in the slaves buffer
* is returned immediately. Slaves are NOT operated in a synchronous way (method to get data from
* slaves is non-blocking).
**/

#ifndef HWACCESS_H
#define HWACCESS_H

#include <boost/asio.hpp>
#include <boost/cstdint.hpp>
#include "hw_thread.h"

namespace tia
{
  class DataPacket;
}

namespace tobiss
{

class XMLParser;

//---------------------------------------------------------------------------------------

/**
* @class HWAccess
*
* @brief Class gathering access to connected hardware.
*
* HWAccess manages every connected hardware and acquires data form master and slaves.
* It is also responsible for synchronisation between connected devices.
* It can hold multiple types of hardware objects, derived from HWThread, whereby
* only ONE master is allowed coincident.
*
* @attention Inappropriate downsamlpling used now!
* @todo Eliminate the master/slave ratio principle!
*/
class HWAccess
{
  public:
    /**
    * @brief Constructor
    *
    * @param[in] io_service Boost::Asio::io_service
    * @param[in] x XMLParser object to gather needed config information
    */
    HWAccess(boost::asio::io_service& io_service, XMLParser& x);

    /**
    * @brief Destructor
    *
    * Deleting dynamically allocated memory for HWThread objects.
    */
    virtual ~HWAccess();

    /**
    * @brief Fill a DataPacket with data from attached hardware synchronous with the master device.
    *
    * Signals a ascending ordered dependent on their SignalType (EEG, EMG,...).
    * SampleNr is automatically increased.
    * DataPacket (holded by hw_access object) is reseted all the time this method is called.
    */
    void fillDataPacket(tia::DataPacket* packet);

    /**
    * @brief Starts the data acquisition.
    */
    void startDataAcquisition();
    /**
    * @brief Stops the data acquisition.
    */
    void stopDataAcquisition();

    /**
    * @brief Get a vector containing all acquired signaltypes.
    *
    * @return vector<uint32_t> The vector contains the signaltypes in ascending order. Signals are
    * stored in the DataPacket with this ordering.
    */
    std::vector<boost::uint32_t> getAcquiredSignalTypes();

    /**
    * @brief Get a vector containing the blocksizes for every acquired signaltype.
    *
    * @return vector<uint32_t> The vector contains the blocksize for every acquired signaltype.
    * Use getAcquiredSignalTypes() to correctly map the blocksizes to their signaltypes!
    */
    std::vector<boost::uint16_t> getBlockSizesPerSignalType();

    /**
    * @brief Get a vector containing the amount of channels for every acquired signaltype.
    *
    * @return vector<uint32_t> The vector contains the amount of channels for every acquired signaltype.
    * Use getAcquiredSignalTypes() to correctly map the nr of channels to their signaltypes!
    */
    std::vector<boost::uint16_t> getNrOfChannelsPerSignalType();

    /**
    * @brief Get a vector containing the sampling rates for every acquired signaltype.
    *
    * @return vector<uint32_t> The vector contains the sampling rates for every acquired signaltype.
    * Use getAcquiredSignalTypes() to correctly map the sampling rates to their signaltypes!
    */
    std::vector<boost::uint32_t> getSamplingRatePerSignalType();

    /**
    * @brief Get the total number of acquired channels.
    *
    * @return uint16_t The total amount of channels acquired.
    */
    boost::uint16_t getNrOfChannels();

    /**
    * @brief Get a map containing the channel-names for every signal type.
    *
    * @return map<uint32_t, vector<string> > Map containing a vector with the channel names for
    * every signal type.
    */
    std::map<boost::uint32_t, std::vector<std::string> > getChannelNames()
      { return(channel_naming_); }

    /**
    * @brief Get the number of connected devices.
    *
    * @return uint16_t The number of connected devices.
    */
    boost::uint16_t getNrOfConnectedDevices()
    {
      if(!master_)
        return(0);
      return(slaves_.size()+1);
    }

    /**
    * @brief Get the masters sampling rate.
    *
    * @return uint32_t The masters sampling rate.
    */
    boost::uint32_t getMastersSamplingRate()
    {
      if(!master_)
        return(0);
      return(master_->getSamplingRate());
    }

    /**
    * @brief Get the masters blocksize.
    *
    * @return uint32_t The masters blocksize.
    */
    boost::uint32_t getMastersBlocksize()
    {
      if(!master_)
        return(0);
      return(master_->getNrOfBlocks());
    }

    /**
    * @brief Get the names to build the respective hardware objects.
    */
    static std::vector<std::string> getPossibleHardwareNames();


//-----------------------------------------------

  private:
    /**
    * @brief Builds a map containing information concerning the acquired data.
    *
    * This method builds an internal map containing the number of channels and the
    * blocksize for every signaltype acquired by connected devices.
    */
    void buildDataInfoMap();

    /**
    * @brief Builds a map containing the sampling rate for every signal type.
    *
    * This method builds an internal map containing the sampling rate
    * for every signaltype acquired by connected devices.
    */
    void buildFsInfoMap();

    /**
    * @brief Do hardware settings.
    *
    * This methods checks hardware settings from hw_access view, e.g. only one master
    * is defined.
    */
    void doHWSetup();
    /**
    * @brief Checks, if multiple masters are defined.
    */
    void checkIfSingleMaster();
    /**
    * @brief Set aperiodic devices in aperiodics_ and remove from slaves_
    */
    void setAperiodics();
    /**
    * @brief Sets a fixed master/slave ratio to ensure a repetitive data stream.
    */
    void setMasterSlaveRatio();
    /**
    * @brief Reports inhomogenous device settings.
    *
    * This method reports an inhomogenous device setting (different signal types
    * from one amplifier) to stout.
    */
    void reportInHomogenousDevice(HWThread* dev);

//-----------------------------------------------

  private:
    /**
    * @brief A vector holding pointers to all connected slave hardware objects.
    */
    std::vector< HWThread* > slaves_;

    /**
    * @brief A vector holding pointers to all connected slave hardware objects.
    */
    std::vector< HWThread* > aperiodics_;

    /**
    * @brief A pointer to the master-device object.
    */
    HWThread* master_;

    /**
    * @brief A pointer to the event-listener object.
    */
    HWThread* event_listener_;

    /**
    * @brief A vector containing sampling rate rations (in unsigned int!!) compared to the master device.
    */
    std::vector<unsigned int> fs_ratio_;
    /**
    * @brief A vector to iterate up to the desired fs_ratio.
    */
    std::vector<unsigned int> sample_it_;  // iteration variable for sample-count

    std::map<boost::uint32_t, std::pair<boost::uint16_t, boost::uint16_t> > data_info_;  ///< map containing ( type,  (nr_ch, blocksize) )
    std::map<boost::uint32_t, double > fs_info_;  ///< map containing ( type, sampling rate )

    std::map<boost::uint32_t, std::vector<std::string> > channel_naming_;  ///< map containing ( type,  channel names )

  #ifdef TIMING_TEST
  private:
    bool  lpt_flag_;
  #endif

};

} // Namespace tobiss

#endif // HWACCESS_H

//---------------------------------------------------------------------------------------

