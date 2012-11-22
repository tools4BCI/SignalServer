#ifndef GDF_WRITER_IMPL_H
#define GDF_WRITER_IMPL_H

#include "filewriter/file_writer.h"
#include "filewriter/file_writer_impl_base.h"
#include <map>
#include "extern/include/libgdf/GDF/Types.h"

namespace gdf
{
  class Writer;
}

//-----------------------------------------------------------------------------

namespace tobiss
{

class GdfWriterImpl : public FileWriterImplBase
{
  public:
    GdfWriterImpl();
    virtual ~GdfWriterImpl();

    virtual void setFilename(std::string filename);
    virtual void open();
    virtual bool isopen();
    virtual void close();
    virtual void flush();

    virtual size_t addNewChannel(std::string label, FileWriterDataTypes::ChannelDataType type,
                                 double sampling_rate,
                                 double dig_min = NAN, double dig_max = NAN,
                                 double phys_min = NAN, double phys_max = NAN);

    // single samples
    virtual void addSample(const size_t channel_idx, const float value);
    virtual void addSample(const size_t channel_idx, const double value);
    virtual void addSample(const size_t channel_idx, const bool value);

    virtual void addSample(const size_t channel_idx, const int value);
    virtual void addSample(const size_t channel_idx, const unsigned int value);

    virtual void addSample(const size_t channel_idx, const short value);
    virtual void addSample(const size_t channel_idx, const unsigned short value);

    virtual void addSample(const size_t channel_idx, const long value);
    virtual void addSample(const size_t channel_idx, const unsigned long value);

    virtual void addSample(const size_t channel_idx, const char value);
    virtual void addSample(const size_t channel_idx, const unsigned char value);

    // vectors
    virtual void addSamples(const size_t channel_idx, const std::vector<float> values);
    virtual void addSamples(const size_t channel_idx, const std::vector<double> values);
    virtual void addSamples(const size_t channel_idx, const std::vector<bool> values);

    virtual void addSamples(const size_t channel_idx, const std::vector<int> values);
    virtual void addSamples(const size_t channel_idx, const std::vector<unsigned int> values);

    virtual void addSamples(const size_t channel_idx, const std::vector<short> values);
    virtual void addSamples(const size_t channel_idx, const std::vector<unsigned short> values);

    virtual void addSamples(const size_t channel_idx, const std::vector<long> values);
    virtual void addSamples(const size_t channel_idx, const std::vector<unsigned long> values);

    virtual void addSamples(const size_t channel_idx, const std::vector<char> values);
    virtual void addSamples(const size_t channel_idx, const std::vector<unsigned char> values);

    // events
    virtual void setEventSampingRate(double fs);

    virtual void addEvent(const size_t position, const double       type);
    virtual void addEvent(const size_t position, const int          type);
    virtual void addEvent(const size_t position, const unsigned int type);
    virtual void addEvent(const size_t position, const std::string& str);

    virtual void addEvent(const size_t position, const double type,
                                       const size_t channel,  const float duration );
    virtual void addEvent(const size_t position, const int    type,
                                       const size_t channel,  const float duration );
    virtual void addEvent(const size_t position, const unsigned int type,
                                       const size_t channel,  const float duration );
    virtual void addEvent(const size_t position, const std::string& str,
                                       const size_t channel,  const float duration );

  private:
    void throwIfOpen(std::string caller);

  private:
    gdf::Writer*                                                gdf_writer_;

    std::map<FileWriterDataTypes::ChannelDataType, gdf::type_id>         types_map_;
    boost::uint32_t                                             max_sampling_rate_;
};

}

//-----------------------------------------------------------------------------

#endif // GDF_WRITER_IMPL_H
