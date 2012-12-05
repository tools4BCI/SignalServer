#include "filewriter/gdf_writer_impl.h"

#include "extern/include/libgdf/GDF/Writer.h"

#include <boost/current_function.hpp>
#include <boost/math/special_functions.hpp>
#include <iostream>
#include <algorithm>
#include <utility>

using std::make_pair;
using std::map;

//-----------------------------------------------------------------------------

enum ChDtTp
{
  INT8,
  UINT8,
  INT16,
  UINT16,
  INT32,
  UINT32,
  INT64,
  UINT64,
  FLOAT,
  DOUBLE
};

namespace tobiss
{

GdfWriterImpl::GdfWriterImpl()
  : FileWriterImplBase(), gdf_writer_(0), max_sampling_rate_(1)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  types_map_.insert(make_pair( FileWriterDataTypes::FLOAT,  gdf::FLOAT32 ));
  types_map_.insert(make_pair( FileWriterDataTypes::DOUBLE, gdf::FLOAT64 ));
  types_map_.insert(make_pair( FileWriterDataTypes::INT8,   gdf::INT8 ));
  types_map_.insert(make_pair( FileWriterDataTypes::UINT8,  gdf::UINT8 ));
  types_map_.insert(make_pair( FileWriterDataTypes::INT16,  gdf::INT16 ));
  types_map_.insert(make_pair( FileWriterDataTypes::UINT16, gdf::UINT16 ));
  types_map_.insert(make_pair( FileWriterDataTypes::INT32,  gdf::INT32 ));
  types_map_.insert(make_pair( FileWriterDataTypes::UINT32, gdf::UINT32 ));
  types_map_.insert(make_pair( FileWriterDataTypes::INT64,  gdf::INT64 ));
  types_map_.insert(make_pair( FileWriterDataTypes::UINT64, gdf::UINT64 ));

  gdf_writer_ = new gdf::Writer();
}

//-----------------------------------------------------------------------------

GdfWriterImpl::~GdfWriterImpl()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  if(gdf_writer_)
  {
    if(gdf_writer_->isOpen())
      gdf_writer_->close();

    delete gdf_writer_;
    gdf_writer_ = 0;
  }
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::setFilename(std::string filename)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  throwIfOpen( std::string(BOOST_CURRENT_FUNCTION) );

  gdf_writer_->setFilename(filename);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::open()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  if(gdf_writer_->isOpen())
    return;

//  gdf_writer_->getHeaderAccess().setRecordDuration( 2* max_sampling_rate_, 2*max_sampling_rate_ );
  gdf_writer_->setEventMode( 1 );     // FIXME: 1 ... EventMode 1 (gdf)


  double systime = boost::numeric_cast<double>( time( NULL ) );
  double tmptime = ( systime/(3600.0*24.0) + 719529.0 ) * pow(2.0,32);
  gdf_writer_->getMainHeader( ).set_recording_start( boost::numeric_cast<gdf::uint64>(tmptime) );

  gdf_writer_->open();
}

//-----------------------------------------------------------------------------

bool GdfWriterImpl::isopen()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif
  return(gdf_writer_->isOpen());
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::close()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  if(gdf_writer_->isOpen())
    gdf_writer_->close();
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::flush()
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif
  gdf_writer_->flush();
}

//-----------------------------------------------------------------------------

size_t GdfWriterImpl::addNewChannel(std::string label, FileWriterDataTypes::ChannelDataType type,
                                    double fs, double dig_min, double dig_max,
                                    double phys_min, double phys_max)
{
  throwIfOpen( std::string(BOOST_CURRENT_FUNCTION) );

  size_t idx = gdf_writer_->getFirstFreeSignalIndex();
  if(!gdf_writer_->createSignal(idx, false))
    throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Can't add channel!" ));

  gdf_writer_->getSignalHeader(idx).set_label(label);

  std::map<FileWriterDataTypes::ChannelDataType, gdf::type_id>::iterator it = types_map_.find(type);

  if(it == types_map_.end())
    throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Can't find datatype!" ));

  gdf_writer_->getSignalHeader(idx).set_datatype(it->second);

  try
  {
    boost::uint32_t fs_tmp = boost::numeric_cast<boost::uint32_t>(fs);
    gdf_writer_->getSignalHeader(idx).set_samplerate(fs_tmp);

    max_sampling_rate_ = std::max(fs_tmp, max_sampling_rate_);

    gdf_writer_->getHeaderAccess().setRecordDuration( 2* max_sampling_rate_, 2*max_sampling_rate_ );
  }
  catch(boost::numeric::bad_numeric_cast& e)
  {
    throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: " + e.what() ) );
  }

  if( (type == FileWriterDataTypes::FLOAT) ||  (type == FileWriterDataTypes::DOUBLE))
  {
    gdf_writer_->getSignalHeader(idx).set_digmin( -250000 );
    gdf_writer_->getSignalHeader(idx).set_digmax( 250000 );
    gdf_writer_->getSignalHeader(idx).set_physmin( -250000 );
    gdf_writer_->getSignalHeader(idx).set_physmax( 250000 );
  }
  else
  {
    if( boost::math::isnan(dig_min)  || boost::math::isnan(dig_max) ||
        boost::math::isnan(phys_min) || boost::math::isnan(phys_max) )
      throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: digital or physical min/max is not correctly set!" ));

    gdf_writer_->getSignalHeader(idx).set_digmin( dig_min );
    gdf_writer_->getSignalHeader(idx).set_digmax( dig_max );
    gdf_writer_->getSignalHeader(idx).set_physmin( phys_min );
    gdf_writer_->getSignalHeader(idx).set_physmax( phys_max );
  }


  return idx;
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const float value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const double value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const bool value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const int value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const unsigned int value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const short value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const unsigned short value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const long value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const unsigned long value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const char value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSample(const size_t channel_idx, const unsigned char value)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->addSamplePhys(channel_idx, value);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::setEventSampingRate(double fs)
{
  throwIfOpen( std::string(BOOST_CURRENT_FUNCTION) );
  gdf_writer_->setEventSamplingRate( fs );

}

//-----------------------------------------------------------------------------

void GdfWriterImpl::throwIfOpen(std::string caller)
{
  if(gdf_writer_->isOpen())
  {
    std::cerr << "Error -- File already/still opened!" << std::endl;
    throw(std::runtime_error( caller + " -- Error: File already/still open!" ));
  }
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<float> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  //gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t channel_idx, const std::vector<double> values)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  gdf_writer_->blitSamplesPhys(channel_idx, values);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<bool> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  // gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<int> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
  //  gdf_writer_->blitSamplesPhys(channel_idx, values);
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<unsigned int> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  // gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<short> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  // gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<unsigned short> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  //  gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<long> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  //gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<unsigned long> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  //gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<char> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  //gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addSamples(const size_t /*channel_idx*/, const std::vector<unsigned char> /*values*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  //gdf_writer_->blitSamplesPhys(channel_idx, values);
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addEvent(const size_t position, const double type)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  try
  {
    gdf::uint16 type_tmp = boost::numeric_cast<gdf::uint16>(type);
    gdf_writer_->addEvent(position, type_tmp);
  }
  catch(boost::numeric::bad_numeric_cast& e)
  {
    throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: " + e.what() ) );
  }
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addEvent(const size_t position, const int type)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  try
  {
    gdf::uint16 type_tmp = boost::numeric_cast<gdf::uint16>(type);
    gdf_writer_->addEvent(position, type_tmp);
  }
  catch(boost::numeric::bad_numeric_cast& e)
  {
    throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: " + e.what() ) );
  }
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addEvent(const size_t position, const unsigned int type)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  try
  {
    gdf::uint16 type_tmp = boost::numeric_cast<gdf::uint16>(type);
    gdf_writer_->addEvent(position, type_tmp);
  }
  catch(boost::numeric::bad_numeric_cast& e)
  {
    throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: " + e.what() ) );
  }
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addEvent(const size_t /*position*/, const std::string& /*str*/)
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addEvent(const size_t /*position*/, const double /*type*/,
                                   const size_t /*channel*/,  const float /*duration*/ )
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif

  //  try
  //  {
  //    gdf::int16 type_tmp = boost::numeric_cast<gdf::int16>(type);
  //    gdf_writer_->addEvent(position, type_tmp,channel, duration)
  //  }
  //  catch(boost::numeric::bad_numeric_cast& e)
  //  {
  //    throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: " + e.what() ) );
  //  }
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet -- Hard coded event-mode set to 1!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addEvent(const size_t /*position*/, const int    /*type*/,
                                   const size_t /*channel*/,  const float /*duration*/ )
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet -- Hard coded event-mode set to 1!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addEvent(const size_t /*position*/, const unsigned int /*type*/,
                                   const size_t /*channel*/,  const float /*duration*/ )
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet -- Hard coded event-mode set to 1!" ) );
}

//-----------------------------------------------------------------------------

void GdfWriterImpl::addEvent(const size_t /*position*/, const std::string& /*str*/,
                                   const size_t /*channel*/,  const float /*duration*/ )
{
  #ifdef DEBUG
    std::cout << BOOST_CURRENT_FUNCTION << std::endl;
  #endif
  throw(std::runtime_error( std::string(BOOST_CURRENT_FUNCTION) + " -- Error: Not supported yet -- Hard coded event-mode set to 1!" ) );
}

//-----------------------------------------------------------------------------

}
