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
* @file kinect.h
* @brief This file includes a class handling handpoint data from a motion sensing system (send over IPC).
**/

#ifndef HANDPOINT_H
#define HANDPOINT_H

#include <boost/cstdint.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "hw_thread.h"
#include "hw_thread_builder.h"


namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class Handpoint
* @brief A class using IPC to receive handpoint data from a motion sensing system
* @todo TODO
*/
class KinectShmReader : public HWThread
{
  public:
    /**
    * @brief Constructor
    */
    KinectShmReader (ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~KinectShmReader();

    /**
    * @brief Method to achieve asynchronous data acquisition (method is non-blocking).
    * @return SampleBlock<double>
    *
    * This method return immediately after calling with a copy of the data stored in the device's
    * buffer.
    * It is called from all slave devices.
    */
    virtual SampleBlock<double> getAsyncData();

    /**
    * @brief Method to start data acquisition.
    */
    virtual void run();

    /**
    * @brief Method to stop data acquisition.
    */
    virtual void stop();

    //-----------------------------------------------
  private:
    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Method to achieve synchronous data acquisition.
    * @todo Implement this method.
    */
    virtual SampleBlock<double> getSyncData()   {return data_; }

    //-----------------------------------------------

    boost::uint32_t old_frame_id_;
    boost::posix_time::ptime last_change_;

    bool connected_, event_reset_needed_;

    std::vector<double> KinectShmReader_data_;

    SampleBlock<double> data_;
    SampleBlock<double> empty_block_;

    boost::interprocess::shared_memory_object *shared_memory_obj_;
    boost::interprocess::mapped_region *region_;

    struct shared_memory_struct
    {
      double x, y, z, confidence;
      boost::uint32_t event, state, frame_id, clients;

      boost::interprocess::interprocess_mutex mutex;
    } *shared_memory_;

    // timeout for IPC in ms
    static const long TIMEOUT = 100;

    // name of shared memory
    static const char *SHARED_MEMORY_NAME;

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<KinectShmReader> FACTORY_REGISTRATOR_;
};

} // Namespace tobiss

#endif // HANDPOINT_H

//-----------------------------------------------------------------------------
