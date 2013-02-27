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
* @file imus.h
* @brief This file includes a class handling IMU Devices.
**/

#ifndef IMUS_H
#define IMUS_H

#ifdef __GNUC__
  #define DISABLE_PLUX_COMPILE
  #define DISABLE_PLUX_REASON "The BioPlux API is only available for MS Windows."
#endif

#include "hw_thread.h"
#include "hw_thread_builder.h"

#include "misc/statistics.h"
#include "misc/databuffer.h"

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class IMUs
* @brief A class to access IMU Devices.
*
* Implements HWThread access to BioPlux.
* Devices are identified by their MAC addresses. (The test device has MAC string set to "Test".)
* If no MAC is specified, the class attempts to autodetect the device.
* The BioPlux API returns a sequence number for each acquired frame (=sample). By tracking the
* sequence number lost samples can be identified. Lost samples are replaced by linear interpolation.
* In asynchroneous (slave) mode, frames are stored in a ring buffer. This inevitably causes some delay.
* When the buffer is full, old frames are overwritten. When reading from an empty buffer, an artificial
* sample is inserted. Under bad timing conditions (imprecise synchronization of devices) the buffer size
* provides a trade-off between frame delay and chance of frame loss.
*/
class IMUs : public HWThread
{
public:

	 /**
    * @brief Constructor
    */
    IMUs (ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~IMUs();
	
	/**
    * @brief Method to achieve synchronous data acquisition (method is blocking).
    * @return SampleBlock<double>
    *
    * This method returns after new data has been acquired, it is operating in blocking mode.
    * It is called only from the master device.
    */
    virtual SampleBlock<double> getSyncData();

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

private:

    /**
    * @brief Start async acquisition.
    *
    * This essentially launches a thread that uses blocking data acquisition to write
    * Data to async_buffer_.
    */
    //void startAsyncAquisition( );

   /**
    * @brief Stop async acquisition.
    * @param[in] blocking bool True causes the function to block until the thread is stopped.
    */
    //void stopAsyncAquisition( bool blocking = true );

    /**
    * @brief Print statistics about data acquisition.
    */
    void printStatistics( );

    /**
    * @brief Set the configuration of the Plux Device with a XML object
    * @param[in] hw ticpp::Element pointing to an \<hardware\> tag in the config file
    * @throw std::invalid_argument if \<channel_settings\> is defined multiple times.
    */
    void setHardware(ticpp::Iterator<ticpp::Element>const &hw);

    /**
    * @brief Set configuration listed in the \<device_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    * @throw ticpp::exception
    */
    virtual void setDeviceSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief Set configuration listed in the \<channel_settings\> section in the XML file.
    * @param[in] hw ticpp::Element pointing to an \<device_settings\> tag in the config file
    */
    virtual void setChannelSettings(ticpp::Iterator<ticpp::Element>const &father);

    /**
    * @brief convert PLUX frames into SampleBlock.
    * @todo implement.
    */
    void convertFrames2SampleBlock( );

    /**
    * @brief Asynchroneously collect data.
    */
    void asyncAcquisitionThread( );

private:
  
    /**
    * @brief Class for validating the Sequence number returned by the PLUX device.
    */
    class SequenceNumber
    {
    public:
      typedef char seqtype;
      SequenceNumber( int s = -1 );
      virtual ~SequenceNumber( ) { }

      bool valid( );

      void setInvalid( ) { seq = -1; }
      
      void operator++( int ); //postfix operator
      void operator--( int ); //postfix operator
      bool operator==( const seqtype &s ) const;
      bool operator>( const seqtype &s ) const;
      bool operator<( const seqtype &s ) const;

      template<typename T> T cast( ) const { return boost::numeric_cast<T>( seq ); }

    private:
      seqtype seq;
    };

    struct Frame
    {
    };

    struct frametype
    {
      /**
      * @brief Constructor (default).
      */
      frametype( ) : frame(), time() { }

      /**
      * @brief Constructor (assigning).
      */
      frametype( Frame f, boost::posix_time::ptime t ) : frame(f), time(t) { }

      /**
      * @brief Constructor (interpolating).
      *
      * The frame is interpolated between a and b, using the sequence numbers.
      */
      frametype( const frametype &a, const frametype &b, const SequenceNumber &seq );

      Frame frame;        ///< BioPlux Frames
      boost::posix_time::ptime time;  ///< Corresponding acquisition time
    };

	struct ImplDetails;

	ImplDetails *impl;

    void waitForNewFrame( );
    
    /**
    * @brief Read most recent data frame and put it into the SampleBlock (data_) at position bidx.
    */
    void getDataFrame( int bidx );

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<IMUs> FACTORY_REGISTRATOR_;

    static const std::string hw_buffersize_;     ///< xml-tag hardware: buffersize

    std::map<std::string, std::string> m_;	///< Attributes map -- to be renamed
    
    std::string devstr_;      ///< Device Identifier
    std::string devinfo_;     ///< Device Description

    boost::posix_time::ptime last_time_;
    frametype last_frame_;        ///< Last frame that was processed
    frametype pending_frame_;     ///< Unprocessed frame
    bool frame_pending_;
    SequenceNumber seq_expected;  ///< Next expected sequence number
    unsigned long long num_frames_total_;  ///< Total number of frames processed

    std::vector<Frame> frames_;   ///< Pre-allocated buffer with BioPlux frames
    std::vector<double> samples_;             ///< Pre-allocated samples for writing to the SampleBlock

    boost::thread async_acquisition_thread_;      ///< Handle to the async acquisition thread
    DataBuffer<frametype> async_buffer_;          ///< Ring buffer for async acquisition (thread safe!)
    DataBuffer<frametype>::size_type buffersize_; ///< Maximum size of the async buffer

    /**
     * @brief Various acquisition statistics.
     */
    struct {
      void reset( )
      {
        time_statistics_.reset( );
        last_printed_ = boost::posix_time::microsec_clock::local_time();
        frames_lost_ = 0;
        frames_repeated_ = 0;
        frames_dropped_ = 0;
      }
      long long frames_lost_, frames_repeated_, frames_dropped_;
      Statistics time_statistics_;
      Statistics rate_statistics_;
      unsigned int statistics_interval_;
      boost::posix_time::ptime last_printed_;
    } statistics_;
};

} // Namespace tobiss

#endif
