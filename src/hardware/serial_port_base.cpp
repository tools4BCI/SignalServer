#include "hardware/serial_port_base.h"

#include <iostream>
#include <boost/bind.hpp>

#define DEFAULT_BAUD_RATE 9600

namespace tobiss
{

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::flush;

//-----------------------------------------------------------------------------

std::map<unsigned int, std::string> AsioSerialPortTypeNames::flow_control_values_;
std::map<unsigned int, std::string> AsioSerialPortTypeNames::stop_bit_values_;
std::map<unsigned int, std::string> AsioSerialPortTypeNames::parity_values_;

//-----------------------------------------------------------------------------

AsioSerialPortTypeNames& AsioSerialPortTypeNames::getInstance()
{
 static AsioSerialPortTypeNames instance;
 return instance;
}

//-----------------------------------------------------------------------------

AsioSerialPortTypeNames::AsioSerialPortTypeNames()
{
  flow_control_values_[1] = "none";
  flow_control_values_[2] = "software";
  flow_control_values_[3] = "hardware";

  stop_bit_values_[1] = "1";
  stop_bit_values_[2] = "1.5";
  stop_bit_values_[3] = "2";

  parity_values_[1] = "none";
  parity_values_[2] = "odd";
  parity_values_[3] = "even";
}

//-----------------------------------------------------------------------------

std::string AsioSerialPortTypeNames::getFlowControlName(unsigned int id)
{
  std::string str;
  str = ((flow_control_values_.find(id) != flow_control_values_.end() )?
         flow_control_values_.find(id)->second : "");
  if(!str.size())
    throw(std::runtime_error("AsioSerialPortTypeNames::getFlowControlName() -- ID not found!"));
  return(str);
}

//-----------------------------------------------------------------------------

std::string AsioSerialPortTypeNames::getStopBitName(unsigned int id)
{
  std::string str;
  str = ( (stop_bit_values_.find(id) != stop_bit_values_.end() )?
           stop_bit_values_.find(id)->second : "");
  if(!str.size())
    throw(std::runtime_error("AsioSerialPortTypeNames::getStopBitName() -- ID not found!"));
  return(str);
}

//-----------------------------------------------------------------------------

std::string AsioSerialPortTypeNames::getParityName(unsigned int id)
{
  std::string str;
  str = ( (parity_values_.find(id) != parity_values_.end() )?
    parity_values_.find(id)->second : ""  );
  if(!str.size())
    throw(std::runtime_error("AsioSerialPortTypeNames::getParityName() -- ID not found!"));
  return(str);
}

//-----------------------------------------------------------------------------

unsigned int AsioSerialPortTypeNames::getFlowControlID(std::string str)
{
  std::map<unsigned int, std::string>::iterator it =
      find_if(flow_control_values_.begin(),
              flow_control_values_.end(),
              std::bind2nd(MapValue(), str) );

  return(it != stop_bit_values_.end() ? it->first :
         throw(std::runtime_error("AsioSerialPortTypeNames::getFlowControlID() -- Name not found " + str)));
}

//-----------------------------------------------------------------------------

unsigned int AsioSerialPortTypeNames::getStopBitID(std::string str)
{
  std::map<unsigned int, std::string>::iterator it =
      find_if(stop_bit_values_.begin(),
              stop_bit_values_.end(),
              std::bind2nd(MapValue(), str) );

  return(it != stop_bit_values_.end() ? it->first :
         throw( std::runtime_error("AsioSerialPortTypeNames::getStopBitID() -- Name not found: " + str)));
}

//-----------------------------------------------------------------------------

unsigned int AsioSerialPortTypeNames::getParityID(std::string str)
{
  std::map<unsigned int, std::string>::iterator it =
      find_if(parity_values_.begin(),
              parity_values_.end(),
              std::bind2nd(MapValue(), str) );

  return(it != stop_bit_values_.end() ? it->first :
         throw(std::runtime_error("AsioSerialPortTypeNames::getParityID() -- Name not found: " + str)));
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


SerialPortBase::SerialPortBase(boost::asio::io_service& io)
  : serial_port_(io), data_available_(true), data_written_(true),
    baud_rate_(0), flow_control_type_(0), parity_(0), stop_bits_(0), character_size_(0),
    asio_types_(AsioSerialPortTypeNames::getInstance())
{

}

//-----------------------------------------------------------------------------

SerialPortBase::~SerialPortBase()
{
  if( !serial_port_.is_open() )
    return;

  boost::system::error_code ec;
  serial_port_.close(ec);
  if(ec)
    std::cerr << "SerialPortBase::Destructor() -- Error closing port: " + port_name_ << std::endl;
}

//-----------------------------------------------------------------------------

void SerialPortBase::setPortName(const std::string& name)
{
  if( name == "" )
    throw(std::runtime_error("SerialPortBase::open() -- No serial port given"));

  port_name_ = name;
}

//-----------------------------------------------------------------------------

void SerialPortBase::open()
{
  if( serial_port_.is_open() )
    throw(std::runtime_error("SerialPortBase::open() -- Port already open: " + port_name_));

  boost::system::error_code ec;
  serial_port_.open(port_name_, ec);

  if(ec)
    throw(std::runtime_error("SerialPortBase::open() -- Error opening serial port: " + port_name_));

  if(!baud_rate_)
  {
    baud_rate_ = DEFAULT_BAUD_RATE;

    boost::asio::serial_port_base::baud_rate br(baud_rate_);
    serial_port_.set_option(br);
  }
}

//-----------------------------------------------------------------------------

void SerialPortBase::close()
{
  if( !serial_port_.is_open() )
    return;

  boost::system::error_code ec;
  serial_port_.close(ec);

  if(ec)
    std::cerr << "SerialPortBase::close() -- Error closing port: " + port_name_ << std::endl;
}

//-----------------------------------------------------------------------------

void SerialPortBase::setBaudRate(const unsigned int rate)
{
  if( !serial_port_.is_open() )
    throw(std::runtime_error("SerialPortBase::setBaudRate() -- Port not opened: " + port_name_));

  boost::asio::serial_port_base::baud_rate br(rate);
  serial_port_.set_option(br);

  baud_rate_ = rate;
}

//-----------------------------------------------------------------------------

void SerialPortBase::setFlowControl(const string& type)
{
  if( !serial_port_.is_open() )
    throw(std::runtime_error("SerialPortBase::setFlowControl() -- Port not opened: " + port_name_));

  if(asio_types_.getFlowControlID(type) == 1)
  {
    boost::asio::serial_port_base::flow_control o(boost::asio::serial_port_base::flow_control::none);
    serial_port_.set_option(o);
  }
  if(asio_types_.getFlowControlID(type) == 2)
  {
    boost::asio::serial_port_base::flow_control o(boost::asio::serial_port_base::flow_control::software);
    serial_port_.set_option(o);
  }
  if(asio_types_.getFlowControlID(type) == 3)
  {
    boost::asio::serial_port_base::flow_control o(boost::asio::serial_port_base::flow_control::hardware);
    serial_port_.set_option(o);
  }
}

//-----------------------------------------------------------------------------

void SerialPortBase::setParity(const string& type)
{
  if( !serial_port_.is_open() )
    throw(std::runtime_error("SerialPortBase::setParity() -- Port not opened: " + port_name_));

  if(asio_types_.getParityID(type) == 1)
  {
    boost::asio::serial_port_base::parity o(boost::asio::serial_port_base::parity::none);
    serial_port_.set_option(o);
  }
  if(asio_types_.getParityID(type) == 2)
  {
    boost::asio::serial_port_base::parity o(boost::asio::serial_port_base::parity::odd);
    serial_port_.set_option(o);
  }
  if(asio_types_.getParityID(type) == 3)
  {
    boost::asio::serial_port_base::parity o(boost::asio::serial_port_base::parity::even);
    serial_port_.set_option(o);
  }
}

//-----------------------------------------------------------------------------

void SerialPortBase::setStopBits(const string& bits)
{
  if( !serial_port_.is_open() )
    throw(std::runtime_error("SerialPortBase::setStopBits() -- Port not opened: " + port_name_));

  if(asio_types_.getStopBitID(bits) == 1)
  {
    boost::asio::serial_port_base::stop_bits sb(boost::asio::serial_port_base::stop_bits::one);
    serial_port_.set_option(sb);
  }
  if(asio_types_.getStopBitID(bits) == 2)
  {
    boost::asio::serial_port_base::stop_bits sb(boost::asio::serial_port_base::stop_bits::onepointfive);
    serial_port_.set_option(sb);
  }
  if(asio_types_.getStopBitID(bits) == 3)
  {
    boost::asio::serial_port_base::stop_bits sb(boost::asio::serial_port_base::stop_bits::two);
    serial_port_.set_option(sb);
  }
}

//-----------------------------------------------------------------------------

void SerialPortBase::setCharacterSize(const unsigned int size)
{
  if( !serial_port_.is_open() )
    throw(std::runtime_error("SerialPortBase::setCharacterSize() -- Port not opened: " + port_name_));

  boost::asio::serial_port_base::character_size cs(size);
  serial_port_.set_option(cs);
}

//-----------------------------------------------------------------------------

void SerialPortBase::handleAsyncRead(const boost::system::error_code& ec,
                                     std::size_t bytes_transferred )
{
  if(ec)
    throw(std::runtime_error("SerialPortBase::handleAsyncRead() -- \
                             Error handling async read -- bytes transferred: " + bytes_transferred));

  data_available_ = true;
}

//-----------------------------------------------------------------------------

void SerialPortBase::handleAsyncWrite(const boost::system::error_code& ec,
                                     std::size_t bytes_transferred )
{
  if(ec)
    throw(std::runtime_error("SerialPortBase::handleAsyncWrite() -- \
                             Error handling async write -- bytes transferred: " + bytes_transferred));

  data_written_ = true;
}

//-----------------------------------------------------------------------------

}  // tobiss
