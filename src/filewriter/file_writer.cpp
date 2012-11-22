#include "filewriter/file_writer.h"
#include "filewriter/gdf_writer_impl.h"

#include <stdexcept>
#include <boost/current_function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include "config/xml_tags.h"

namespace tobiss
{

//-----------------------------------------------------------------------------

FileWriter::FileWriter(std::string& filetype)
  : impl_(0), filetype_(filetype),
    file_exists_behaviour_(NewFile),
    current_file_nr_(1)
{
    if(filetype == "gdf")
    {
      impl_ = new GdfWriterImpl;
    }
    else
      throw(std::invalid_argument( std::string( BOOST_CURRENT_FUNCTION ) + " -- Error: FileWriter Type not recognized!" ));

}

//-----------------------------------------------------------------------------

FileWriter::~FileWriter()
{
  if(impl_)
  {
    delete impl_;
    impl_ = 0;
  }
}

//-----------------------------------------------------------------------------

void FileWriter::setFilename(std::string filename)
{
  std::string filename_without_extension = filename.substr(0, filename.rfind("."));

  filename_ = filename_without_extension;
}

//-----------------------------------------------------------------------------

void FileWriter::setFilepath(std::string path_str)
{
  namespace fs = boost::filesystem;

  try
  {
    fs::path path(path_str);
    path = boost::filesystem::system_complete(path);

    if(!fs::exists(path))
      fs::create_directory(path);

    store_path_ = path.string();
  }
  catch(fs::filesystem_error& e)
  {
    throw(std::invalid_argument( std::string(BOOST_CURRENT_FUNCTION) + " -- Error creating path: " + e.what() ));
  }
}

//-----------------------------------------------------------------------------

void FileWriter::setBehaviourIfFileExists(FileExistsBehaviour behaviour)
{
  file_exists_behaviour_ = behaviour;
}

//-----------------------------------------------------------------------------

void FileWriter::setNewFileAppendString(std::string append_str)
{
  new_file_append_str_ = append_str;
}

//-----------------------------------------------------------------------------

void FileWriter::open()
{
  boost::filesystem::path path(store_path_ + "/" );
  path += filename_;

  if(file_exists_behaviour_ == OverWrite)
  {
    path += "." + filetype_;

    if(boost::filesystem::exists(path))
      boost::filesystem::remove(path);
  }
  else  // NewFile
  {
    boost::filesystem::path tmp_path;
    do
    {
      tmp_path = path;

      tmp_path += new_file_append_str_;
      tmp_path += boost::lexical_cast<std::string>(current_file_nr_++);
      tmp_path += "." + filetype_;
    }
    while(boost::filesystem::exists(tmp_path));

    path = tmp_path;
  }

  impl_->setFilename(path.string());
  impl_->open();
}

//-----------------------------------------------------------------------------

bool FileWriter::isopen()
{
  return(impl_->isopen());
}

//-----------------------------------------------------------------------------

void FileWriter::close()
{
  impl_->close();
}

//-----------------------------------------------------------------------------

void FileWriter::flush()
{
  impl_->flush();
}

//-----------------------------------------------------------------------------

size_t FileWriter::addNewChannel(std::string label, FileWriterDataTypes::ChannelDataType type,
                                 double sampling_rate,
                                 double dig_min, double dig_max,
                                 double phys_min, double phys_max)
{
  return(impl_->addNewChannel( label, type, sampling_rate,
                               dig_min, dig_max, phys_min, phys_max ));
}



//-----------------------------------------------------------------------------


}
