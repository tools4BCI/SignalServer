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
* @file plux.h
* @brief This file includes a class handling BioPlux Devices.
**/

#ifndef PLUX_H
#define PLUX_H

#include "hw_thread.h"
#include "hw_thread_builder.h"

#include "misc/statistics.h"
#include "misc/databuffer.h"

#include "BioPlux.h"

#include <boost/thread.hpp>

// Begin a PLUX try block
#define PLUX_TRY try

// Catch a PLUX exception and throw a std::runtime_error
#define PLUX_THROW catch( BP::Err err ) { rethrowPluxException( err, true ); }

// Catch a PLUX exception and print the message
#define PLUX_CATCH catch( BP::Err err ) { rethrowPluxException( err, false ); }

namespace tobiss
{
//-----------------------------------------------------------------------------

/**
* @class Plux
* @brief A class to access BioPlux Devices.
* @todo Everything.
*/
class Plux : public HWThread
{
public:

	 /**
    * @brief Constructor
    */
    Plux (ticpp::Iterator<ticpp::Element> hw);

    /**
    * @brief Destructor
    */
    virtual ~Plux();
	
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

    void startAsyncAquisition( size_t buffer_size );

    void stopAsyncAquisition( bool blocking = true );

    /**
    * @brief Print statistics of time delays in async acquisition mode
    */
    void printAsyncStatistics( );

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
    * @brief Automatically selects an available PLUX device.
    * @return std::string MAC adress of available device
    * @todo What if there is more than one device?.
    */
    static std::string findDevice( );

    /**
    * @brief Throws a standard exception with information obtained from a BioPlux excaption.
    */
    static void rethrowPluxException(  BP::Err &err, bool do_throw );

private:

    class SequenceNumber
    {
    public:
      typedef char seqtype;
      SequenceNumber( int s = -1 );
      virtual ~SequenceNumber( ) { }

      bool valid( );
      
      void operator++( int ); //prefix operator
      void operator--( int ); //prefix operator
      bool operator==( const seqtype &s ) const;
      bool operator>( const seqtype &s ) const;
      bool operator<( const seqtype &s ) const;

      template<typename T> T cast( ) const { return boost::numeric_cast<T>( seq ); }

    private:
      seqtype seq;
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
      frametype( BP::Device::Frame f, boost::posix_time::ptime t ) : frame(f), time(t) { }

      /**
      * @brief Constructor (interpolating).
      */
      frametype( const frametype &a, const frametype &b, const SequenceNumber &seq )
      {        
        double factor = (seq.cast<double>()-a.frame.seq)/(b.frame.seq-a.frame.seq);

        time = a.time + boost::posix_time::microseconds( (b.time - a.time).total_microseconds() * factor );

        for( int i=0; i<8; i++ )
          frame.an_in[i] = a.frame.an_in[i] + boost::numeric_cast<int>( (b.frame.an_in[i] - a.frame.an_in[i]) * factor );

        if( factor < 0.5 )
          frame.dig_in = a.frame.dig_in;
        else
          frame.dig_in = b.frame.dig_in;

        frame.seq = seq.cast<BYTE>();
      }

      BP::Device::Frame frame;
      boost::posix_time::ptime time;
    };

    static const HWThreadBuilderTemplateRegistratorWithoutIOService<Plux> FACTORY_REGISTRATOR_;

    /**
    * @brief Asynchroneously collect data.
    */
    void asyncAcquisitionThread( );

    /**
    * @brief Check if the frame's sequence number correctly increments.
    * @return bool true if everything is OK.
    */
    int checkSequenceNumber( const BYTE id );


    std::map<std::string, std::string> m_;	/// Attributes map -- to be renamed
    BP::Device *device_;

    std::string devstr_;
    std::string devinfo_;

    BYTE last_frame_seq_;
    frametype last_frame_;
    bool first_frame_;
    SequenceNumber seq_expected;

    std::vector<BP::Device::Frame> frames_;
    std::vector<unsigned int> frame_flags_;
    std::vector<double> samples_;
    DataBuffer<frametype> async_buffer_;

    boost::thread async_acquisition_thread_;
    long long frames_lost_, frames_repeated_, frames_dropped_;
    DataBuffer<frametype>::size_type frames_buffered_;

    Statistics time_statistics_;
    unsigned int statistics_interval_;
};

} // Namespace tobiss

#endif
