#include "filereading/data_file_handler.h"

namespace tobiss
{
using std::cout;
using std::endl;

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;

using boost::lexical_cast;
using boost::bad_lexical_cast;

//-----------------------------------------------------------------------------

DataFileHandler::DataFileHandler(boost::asio::io_service& io, std::map<std::string, std::string> config)
  :io_service_(io), config_(config), td_(0)
{

  // check config (file present)

  // get Object from FileFactory
  // set configuration

}

//-----------------------------------------------------------------------------

void DataFileHandler::run()
{
  #ifdef DEBUG
    cout << "FileReader: run" << endl;
  #endif

}


//-----------------------------------------------------------------------------

void DataFileHandler::stop()
{
  #ifdef DEBUG
    cout << "FileReader: stop" << endl;
  #endif

}

//-----------------------------------------------------------------------------

DataPacket DataFileHandler::getDataPacket()
{
  #ifdef DEBUG
    cout << "FileReader: getSyncData" << endl;
  #endif

  DataPacket packet;

  return(packet);
}



//-----------------------------------------------------------------------------

void DataFileHandler::readSamples()
{
  #ifdef DEBUG
    cout << "FileReader: readSamples" << endl;
  #endif

}

//-----------------------------------------------------------------------------

} // Namespace tobiss
