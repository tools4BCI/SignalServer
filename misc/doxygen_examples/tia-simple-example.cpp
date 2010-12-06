
/**
* @file tia-simple-example.cpp
*
* @brief The File demonstrates how to use the TiA server.
**/

// STL
#include <map>
#include <vector>
#include <string>

//boost
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>

//TiA
#include "tia/tia_server.h"
#include "tia/defines.h"
#include "tia/data_packet.h"

int main(int argc, const char* argv[])
{
  try
  {
    boost::asio::io_service io_service;

    tobiss::TiAServer server(io_service);

    // * set up data acquisition system
    // * gather needed information for the TiA server

    // the following values are only used for demonstration purpose

    //--------------------------------------
    //------ hardware related config -------
    //--------------------------------------

    // the number of signal types has to be determined automatically
    // from the data acquisition part
    static const int nr_of_signal_types = 3;

    std::vector< boost::uint32_t > signal_types(nr_of_signal_types);

    // signal types defined in tia/defines.h
    signal_types.at(0) = SIG_EEG;
    signal_types.at(1) = SIG_EOG;
    signal_types.at(2) = SIG_BP;

    //lets make the EEG data acquisition device be the master
    boost::uint32_t master_blocksize = 2;
    boost::uint32_t master_sampling_rate = 512;

    std::vector< boost::uint32_t > sampling_rates(nr_of_signal_types);
    sampling_rates.at(0) = 512;  // ... MASTER
    sampling_rates.at(1) = 512;
    sampling_rates.at(2) = 64;

    std::vector< boost::uint16_t > block_sizes(nr_of_signal_types);
    block_sizes.at(0) = 2;       // ... MASTER
    block_sizes.at(1) = 4;
    block_sizes.at(2) = 1;

    // ****** VERY IMPORTANT: ******
    // The master the device MUST have the highest "virtual" sampling rate.
    // virtual sampling rate: fs_master / bs_master  >  fs_slaves / bs/slaves

    // IMPORTANT: The channel naming map implicitly includes the number of
    //            channels per signal type
    std::map< boost::uint32_t, std::vector< std::string > >  channel_names;

    // lets use 3 EEG channels
    std::vector< std::string >  eeg_channel_names(3);
    eeg_channel_names.at(0) = "C3";
    eeg_channel_names.at(1) = "Cz";
    eeg_channel_names.at(2) = "C4";

    // lets use 4 EOG channels and just name them "eeg"
    std::vector< std::string >  eog_channel_names(4,"eog");

    // lets use 2 HR channels and just name them "eeg"
    std::vector< std::string >   hr_channel_names(2,"hr");

    channel_names.insert( std::make_pair(SIG_EEG, eeg_channel_names ));
    channel_names.insert( std::make_pair(SIG_EOG, eog_channel_names ));
    channel_names.insert( std::make_pair( SIG_HR, hr_channel_names  ));

    // ****  necessary functions to configure the TiA server
    server.setMasterBlocksize(    master_blocksize );
    server.setMasterSamplingRate( master_sampling_rate );

    server.setAcquiredSignalTypes( signal_types );
    server.setBlockSizesPerSignalType( block_sizes );
    server.setSamplingRatePerSignalType( sampling_rates );

    server.setChannelNames( channel_names );

    //--------------------------------------
    //--- TiA Server and Subject config ----
    //--------------------------------------

    // it is intended to get rid of the usage of the constants class;
    // the interface to configure this part might change in future (sorry)

    std::map< std::string, std::string >  server_settings;

    // set the TCP control connection port
    server_settings.insert(
        std::make_pair(tobiss::Constants::ss_ctl_port, "9000") );
    // set the UDP broadcasting port
    server_settings.insert(
        std::make_pair(tobiss::Constants::ss_udp_port, "9998") );
    // set the UDP broadcasting subnet
    server_settings.insert(
        std::make_pair(tobiss::Constants::ss_udp_bc_addr, "192.168.1.255") );

    std::map< std::string, std::string >  subject_info;
    subject_info.insert(
        std::make_pair(tobiss::Constants::s_id,         "aty10f") );
    subject_info.insert(
        std::make_pair(tobiss::Constants::s_first_name, "Jane") );
    subject_info.insert(
        std::make_pair(tobiss::Constants::s_surname,    "Doe") );
    subject_info.insert(
        std::make_pair(tobiss::Constants::s_sex,        "f") );
    subject_info.insert(
        std::make_pair(tobiss::Constants::s_birthday,   " 31.12.1900") ); // DD.MM.YYYY


    // ****  necessary function to configure the TiA server
    server.initialize( subject_info,  server_settings );

    // start the tread
    boost::thread server_thread(
      boost::bind(&boost::asio::io_service::run, &io_service) );


    // build  your own loop to send the data packets ;-)
    for(unsigned int n = 0; n < 100; n++)
    {
      tobiss::DataPacket p;
      // p = YOUR_HARDWARE_ACQ_SYSTEM.getDataPacket;

      // send the data packet to all connected clients
      server.sendDataPacket(p);
    }

    // -------------------------------------
    // clean up

    io_service.stop();
    // done

  }
  // possible exceptions
  catch(std::exception& e)
  {
  }
  catch(boost::exception& e)
  {
  }
  catch(...)
  {
  }

}

