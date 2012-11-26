#ifndef FILE_WRITER_IMPL_BASE_H
#define FILE_WRITER_IMPL_BASE_H

#include "filewriter/file_writer_data_types.h"

#include <string>
#include <vector>
#include <boost/cstdint.hpp>

#include <boost/current_function.hpp>
#include <iostream>
#include <limits>
#include <cmath>


namespace tobiss
{

//-----------------------------------------------------------------------------

class FileWriterImplBase
{
  public:
    virtual ~FileWriterImplBase();

    virtual void setFilename(std::string filename) = 0;
    virtual void open() = 0;
    virtual bool isopen() = 0;
    virtual void close() = 0;
    virtual void flush() = 0;

    virtual size_t addNewChannel(std::string label, FileWriterDataTypes::ChannelDataType type,
                                 double sampling_rate,
                                 double dig_min = std::numeric_limits<double>::quiet_NaN(), double dig_max = std::numeric_limits<double>::quiet_NaN(),
                                 double phys_min = std::numeric_limits<double>::quiet_NaN(), double phys_max = std::numeric_limits<double>::quiet_NaN()) = 0;

    // single samples
    virtual void addSample(const size_t channel_idx, const float value) = 0;
    virtual void addSample(const size_t channel_idx, const double value) = 0;
    virtual void addSample(const size_t channel_idx, const bool value) = 0;

    virtual void addSample(const size_t channel_idx, const int value) = 0;
    virtual void addSample(const size_t channel_idx, const unsigned int value) = 0;

    virtual void addSample(const size_t channel_idx, const short value) = 0;
    virtual void addSample(const size_t channel_idx, const unsigned short value) = 0;

    virtual void addSample(const size_t channel_idx, const long value) = 0;
    virtual void addSample(const size_t channel_idx, const unsigned long value) = 0;

    virtual void addSample(const size_t channel_idx, const char value) = 0;
    virtual void addSample(const size_t channel_idx, const unsigned char value) = 0;

    // vectors
    virtual void addSamples(const size_t channel_idx, const std::vector<float> values) = 0;
    virtual void addSamples(const size_t channel_idx, const std::vector<double> values) = 0;
    virtual void addSamples(const size_t channel_idx, const std::vector<bool> values) = 0;

    virtual void addSamples(const size_t channel_idx, const std::vector<int> values) = 0;
    virtual void addSamples(const size_t channel_idx, const std::vector<unsigned int> values) = 0;

    virtual void addSamples(const size_t channel_idx, const std::vector<short> values) = 0;
    virtual void addSamples(const size_t channel_idx, const std::vector<unsigned short> values) = 0;

    virtual void addSamples(const size_t channel_idx, const std::vector<long> values) = 0;
    virtual void addSamples(const size_t channel_idx, const std::vector<unsigned long> values) = 0;

    virtual void addSamples(const size_t channel_idx, const std::vector<char> values) = 0;
    virtual void addSamples(const size_t channel_idx, const std::vector<unsigned char> values) = 0;

    // events
    virtual void setEventSampingRate(double fs) = 0;

    virtual void addEvent(const size_t position, const double       type) = 0;
    virtual void addEvent(const size_t position, const int          type) = 0;
    virtual void addEvent(const size_t position, const unsigned int type) = 0;
    virtual void addEvent(const size_t position, const std::string& str)  = 0;

    virtual void addEvent(const size_t position, const double type,
                                       const size_t channel,  const float duration ) = 0;
    virtual void addEvent(const size_t position, const int    type,
                                       const size_t channel,  const float duration ) = 0;
    virtual void addEvent(const size_t position, const unsigned int type,
                                       const size_t channel,  const float duration ) = 0;
    virtual void addEvent(const size_t position, const std::string& str,
                                       const size_t channel,  const float duration ) = 0;

  protected:
    FileWriterImplBase();

};

}

//-----------------------------------------------------------------------------

#endif // FILE_WRITER_IMPL_BASE_H
