#ifndef FILE_WRITER_H
#define FILE_WRITER_H

#include <vector>
#include <string>
#include <cmath>

#include <boost/cstdint.hpp>
#include "filewriter/file_writer_data_types.h"
#include "filewriter/file_writer_impl_base.h"

//-----------------------------------------------------------------------------
namespace tobiss
{

class FileWriter
{
  public:
    enum WriterType
    {
      GdfWriter, EdfWriter,BkrWriter,BCI2000Writer
    };

    enum FileExistsBehaviour
    {
      OverWrite, NewFile
    };

    FileWriter(std::string& filetype);
    virtual ~FileWriter();

    void setFilename(std::string filename);
    void setFilepath(std::string path);

    void setBehaviourIfFileExists(FileExistsBehaviour behaviour);
    void setNewFileAppendString(std::string append_str);

    void open();
    bool isopen();
    void close();
    void flush();

    size_t addNewChannel(std::string label, FileWriterDataTypes::ChannelDataType type,
                         double sampling_rate,
                         double dig_min = NAN, double dig_max = NAN,
                         double phys_min = NAN, double phys_max = NAN);

    template<typename T> void addSample(const size_t channel_idx, const T value);
    template<typename T> void addSamples(const size_t channel_idx, const std::vector<T> values);

    void setEventSamplingRate(double fs);

    template<typename T> void addEvent(const size_t position, const T type);
    template<typename T> void addEvent(const size_t position, const T type,
                                       const size_t channel,  const boost::uint32_t duration );


  private:
    FileWriterImplBase*       impl_;
    std::string               filetype_;
    std::string               filename_;
    std::string               store_path_;

    FileExistsBehaviour       file_exists_behaviour_;
    std::string               new_file_append_str_;
    unsigned int              current_file_nr_;

};

//-----------------------------------------------------------------------------

template<typename T> void FileWriter::addSample(const size_t channel_idx, const T value)
{
  impl_->addSample(channel_idx,value);
}

//-----------------------------------------------------------------------------

template<typename T> void FileWriter::addSamples(const size_t channel_idx, const std::vector<T> values)
{
  impl_->addSample(channel_idx,values);
}

//-----------------------------------------------------------------------------

template<typename T> void FileWriter::addEvent(const size_t position, const T type)
{
  impl_->addEvent(position, type);
}

//-----------------------------------------------------------------------------

template<typename T> void FileWriter::addEvent(const size_t position, const T type,
                     const size_t channel,  const boost::uint32_t duration )
{
  impl_->addEvent(position, type, channel, duration);
}

//-----------------------------------------------------------------------------

}

//-----------------------------------------------------------------------------


#endif // FILE_WRITER_H
