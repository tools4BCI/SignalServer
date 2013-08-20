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
* @file TMSi.h
* @brief This file includes a class handling BioTMSi Devices.
**/

#ifndef TMSi_H
#define TMSi_H

#ifdef __GNUC__
  #define DISABLE_TMSI_COMPILE
  #define DISABLE_TMSI_REASON "The TMSi API is only available for MS Windows."
#endif

#include "hw_thread.h"
#include "hw_thread_builder.h"

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class TMSi
* @brief A class to access BioTMSi Devices.
*
* Implements HWThread access to BioTMSi.
* Devices are identified by their MAC addresses. (The test device has MAC string set to "Test".)
* If no MAC is specified, the class attempts to autodetect the device.
* The BioTMSi API returns a sequence number for each acquired frame (=sample). By tracking the
* sequence number lost samples can be identified. Lost samples are replaced by linear interpolation.
* In asynchroneous (slave) mode, frames are stored in a ring buffer. This inevitably causes some delay.
* When the buffer is full, old frames are overwritten. When reading from an empty buffer, an artificial
* sample is inserted. Under bad timing conditions (imprecise synchronization of devices) the buffer size
* provides a trade-off between frame delay and chance of frame loss.
*/
class TMSi : public HWThread
{
public:

	 /**
    * @brief Constructor
    */
    TMSi (ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~TMSi();
	
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
    void startAsyncAquisition( );

   /**
    * @brief Stop async acquisition.
    * @param[in] blocking bool True causes the function to block until the thread is stopped.
    */
    void stopAsyncAquisition( bool blocking = true );

    /**
    * @brief Print statistics about data acquisition.
    */
    void printStatistics( );

    /**
    * @brief Set the configuration of the TMSi Device with a XML object
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
    * @brief convert TMSi frames into SampleBlock.
    * @todo implement.
    */
    void convertFrames2SampleBlock( );

    /**
    * @brief Automatically selects an available TMSi device.
    * @return std::string MAC adress of available device
    * @todo What if there is more than one device?.
    */
    static std::string findDevice( );

    /**
    * @brief Asynchroneously collect data.
    */
    void asyncAcquisitionThread( );

private:
  
    /**
    * @brief Class for validating the Sequence number returned by the TMSi device.
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

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<TMSi> FACTORY_REGISTRATOR_;

    std::vector<double> samples_;             ///< Pre-allocated samples for writing to the SampleBlock

    std::map<std::string, std::string> m_;	///< Attributes map -- to be renamed

    boost::thread async_acquisition_thread_;      ///< Handle to the async acquisition thread
};

} // Namespace tobiss

#endif
