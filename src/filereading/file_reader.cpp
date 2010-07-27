#include "filereading/data_file_handler.h"

namespace tobiss
{


//-----------------------------------------------------------------------------

FileReader::FileReader()
{

}

//-----------------------------------------------------------------------------

void FileReader::run()
{
  #ifdef DEBUG
    cout << "FileReader: run" << endl;
  #endif

}


//-----------------------------------------------------------------------------

void FileReader::stop()
{
  #ifdef DEBUG
    cout << "FileReader: stop" << endl;
  #endif

}

//-----------------------------------------------------------------------------

DataPacket FileReader::getDataPacket()
{
  #ifdef DEBUG
    cout << "FileReader: getSyncData" << endl;
  #endif

  DataPacket packet;

  return(packet);
}



//-----------------------------------------------------------------------------

void FileReader::readSamples()
{
  #ifdef DEBUG
    cout << "FileReader: readSamples" << endl;
  #endif

}

//-----------------------------------------------------------------------------

} // Namespace tobiss
