#include "filereading/file_reader_factory.h"

namespace tobiss
{

//-----------------------------------------------------------------------------

FileReaderFactory::FileReaderFactory()
{

  // build map with all supported dataformats and their respective "basic" objects

}

//-----------------------------------------------------------------------------

FileReaderFactory::~FileReaderFactory()
{

  // delete all FilerReader Objects in the map

}

//-----------------------------------------------------------------------------

FileReader* FileReaderFactory::getFileReader(std::string filetype)
{

  // build map with all supported dataformats and their respective "basic" objects

  return(0);
}

//-----------------------------------------------------------------------------


} // Namespace tobiss
