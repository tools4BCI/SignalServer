
/**
* @file file_reader_factory.h
*
* @brief
*
**/

#ifndef FILEREADERFACTORY_H
#define FILEREADERFACTORY_H

#include <map>
#include <string>

#include "filereading/file_reader.h"

namespace tobiss
{

//-----------------------------------------------------------------------------
/**
* @class FileReaderFactory
*
* @brief
*
*/
class FileReaderFactory
{
  public:
    /**
    *
    */
    FileReaderFactory();

    /**
    * @brief Destructor
    */
    virtual ~FileReaderFactory();


    FileReader* getFileReader(std::string filetype);


//-----------------------------------------------

  private:

    std::map<std::string, FileReader*>   file_readers_;

};

} // Namespace tobiss

#endif // FILEREADERFACTORY_H

//-----------------------------------------------------------------------------
