#include "hardware/serial_port_base.h"

#include <iostream>
#include <boost/bind.hpp>

#define DEFAULT_BAUD_RATE 9600

namespace tobiss
{

//-----------------------------------------------------------------------------

std::map<unsigned int, std::string> AsioSerialPortTypeNames::flow_control_values_;
std::map<unsigned int, std::string> AsioSerialPortTypeNames::stop_bit_values_;
std::map<unsigned int, std::string> AsioSerialPortTypeNames::parity_values_;

//-----------------------------------------------------------------------------

AsioSerialPortTypeNames& AsioSerialPortTypeNames::getInstance()
{
 static AsioSerialPortTypeNames instance;
 return instance;
};

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
  return((flow_control_values_.find(id) != flow_control_values_.end() )?
         flow_control_values_.find(id)->second:
         throw(std::runtime_error("AsioSerialPortTypeNames::getFlowControlName() -- ID not found!")));
}

//-----------------------------------------------------------------------------

std::string AsioSerialPortTypeNames::getStopBitName(unsigned int id)
{
   return( (stop_bit_values_.find(id) != stop_bit_values_.end() )?
           stop_bit_values_.find(id)->second:
           throw(std::runtime_error("AsioSerialPortTypeNames::getStopBitName() -- ID not found!")));
}

//-----------------------------------------------------------------------------

std::string AsioSerialPortTypeNames::getParityName(unsigned int id)
{
  return( (parity_values_.find(id) != parity_values_.end() )?
           parity_values_.find(id)->second:
           throw(std::runtime_error("AsioSerialPortTypeNames::getParityName() -- ID not found!")));
}

//-----------------------------------------------------------------------------

unsigned int AsioSerialPortTypeNames::getFlowControlID(std::string str)
{
  std::map<unsigned int, std::string>::iterator it =
      find_if(flow_control_values_.begin(),
              flow_control_values_.end(),
              std::bind2nd(MapValue(), str) );

  return(it != stop_bit_values_.end() ? it->first :
         throw(std::runtime_error("AsioSerialPortTypeNames::getFlowControlID() -- Name not found!")));
}

//-----------------------------------------------------------------------------

unsigned int AsioSerialPortTypeNames::getStopBitID(std::string str)
{
  std::map<unsigned int, std::string>::iterator it =
      find_if(stop_bit_values_.begin(),
              stop_bit_values_.end(),
              std::bind2nd(MapValue(), str) );

  return(it != stop_bit_values_.end() ? it->first :
        throw( std::runtime_error("AsioSerialPortTypeNames::getStopBitID() -- Name not found!")));
}

//-----------------------------------------------------------------------------

unsigned int AsioSerialPortTypeNames::getParityID(std::string str)
{
  std::map<unsigned int, std::string>::iterator it =
      find_if(parity_values_.begin(),
              parity_values_.end(),
              std::bind2nd(MapValue(), str) );

  return(it != stop_bit_values_.end() ? it->first :
         throw(std::runtime_error("AsioSerialPortTypeNames::getParityID() -- Name not found!")));
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


SerialPortBase::SerialPortBase(boost::asio::io_service& io, XMLParser& parser)
  : HWThread(parser), serial_port_(io), data_available_(true), data_written_(true),
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

void SerialPortBase::open(const std::string port_name)
{
  if( serial_port_.is_open() )
    throw(std::runtime_error("SerialPortBase::open() -- Port already open: " + port_name));

  if(!baud_rate_)
  {
    baud_rate_ = DEFAULT_BAUD_RATE;

    boost::asio::serial_port_base::baud_rate br(baud_rate_);
    serial_port_.set_option(br);

    boost::asio::serial_port_base::flow_control fc;
    boost::asio::serial_port_base::parity pa;
    boost::asio::serial_port_base::stop_bits sb;
    boost::asio::serial_port_base::character_size cs;

    serial_port_.get_option(fc);
    flow_control_type_ = fc.value();

    serial_port_.get_option(pa);
    parity_ = pa.value();

    serial_port_.get_option(sb);
    stop_bits_ = sb.value();

    serial_port_.get_option(cs);
    character_size_ =  cs.value();

    std::cout << "Setting serial port " + port_name + " to default values:" << std::endl;
    std::cout <<  " -- baud rate: " << baud_rate_ << "; ";
    std::cout <<  "flow control: " << asio_types_.getFlowControlName(flow_control_type_) << "; ";
    std::cout <<  "parity: " << asio_types_.getParityName(parity_) << "; ";
    std::cout <<  "stop bits: " << asio_types_.getStopBitName(stop_bits_) << "; ";
    std::cout <<  "character size: " << character_size_;
    std::cout << std::endl;

  }

  serial_port_.open(port_name);
  port_name_ = port_name;
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
  boost::asio::serial_port_base::baud_rate br(rate);
  serial_port_.set_option(br);

  baud_rate_ = rate;
}

//-----------------------------------------------------------------------------

void SerialPortBase::setFlowControl(const string& type)
{
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
  if(asio_types_.getStopBitID(type) == 1)
  {
    boost::asio::serial_port_base::parity o(boost::asio::serial_port_base::parity::none);
    serial_port_.set_option(o);
  }
  if(asio_types_.getStopBitID(type) == 2)
  {
    boost::asio::serial_port_base::parity o(boost::asio::serial_port_base::parity::odd);
    serial_port_.set_option(o);
  }
  if(asio_types_.getStopBitID(type) == 3)
  {
    boost::asio::serial_port_base::parity o(boost::asio::serial_port_base::parity::even);
    serial_port_.set_option(o);
  }
}

//-----------------------------------------------------------------------------

void SerialPortBase::setStopBits(const string& bits)
{
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
  boost::asio::serial_port_base::character_size cs(size);
  serial_port_.set_option(cs);
}

//-----------------------------------------------------------------------------

void SerialPortBase::sync_read(std::vector<char>& values)
{
  boost::asio::read(serial_port_, boost::asio::buffer(values));
}

//-----------------------------------------------------------------------------

void SerialPortBase::sync_read(std::vector<char>& values, unsigned int bytes_to_receive)
{
  boost::system::error_code ec;

  boost::asio::read(serial_port_, boost::asio::buffer(values),
                    boost::asio::transfer_at_least(bytes_to_receive), ec);

  if(ec)
    throw(std::runtime_error("SerialPortBase::sync_read() -- \
                             Error reading serial port -- bytes: " + bytes_to_receive));
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

void SerialPortBase::async_read(std::vector<char>& values)
{
  if(!data_available_)
    throw(std::logic_error("SerialPortBase::async_read() -- Still waiting for new data ...") );

  data_available_ = false;
  boost::asio::async_read(serial_port_,
                          boost::asio::buffer(values),
                          boost::bind(&SerialPortBase::handleAsyncRead,
                                      this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred)
                          );
}

//-----------------------------------------------------------------------------

void SerialPortBase::async_read(std::vector<char>& values, unsigned int bytes_to_receive)
{
  if(!data_available_)
    throw(std::logic_error("SerialPortBase::async_read() -- Still waiting for new data ...") );

  data_available_ = false;
  boost::asio::async_read(serial_port_,
                          boost::asio::buffer(values),
                          boost::asio::transfer_at_least(bytes_to_receive),
                          boost::bind(&SerialPortBase::handleAsyncRead,
                                      this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred)
                          );
}

//-----------------------------------------------------------------------------

void SerialPortBase::sync_write(std::vector<char>& values)
{
  boost::asio::write(serial_port_, boost::asio::buffer(values));
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

void SerialPortBase::async_write(std::vector<char>& values)
{
  if(!data_written_)
    throw(std::logic_error("SerialPortBase::async_read() -- Still writing data ...") );

  data_written_ = false;
  boost::asio::async_write(serial_port_,
                          boost::asio::buffer(values),
                          boost::bind(&SerialPortBase::handleAsyncWrite,
                                      this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred)
                          );
}

//-----------------------------------------------------------------------------

}  // tobiss
