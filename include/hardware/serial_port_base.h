#ifndef SERIALPORTBASE_H
#define SERIALPORTBASE_H

#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

#include <string>
#include <vector>
#include <map>
#include <functional>

//#include "hw_thread.h"

namespace tobiss
{


class AsioSerialPortTypeNames
{
  public:
    ~AsioSerialPortTypeNames()    {  }
    static AsioSerialPortTypeNames& getInstance();

    std::string getFlowControlName(unsigned int id);
    std::string getStopBitName(unsigned int id);
    std::string getParityName(unsigned int id);

    unsigned int getFlowControlID(std::string str);
    unsigned int getStopBitID(std::string);
    unsigned int getParityID(std::string);

  private:
    AsioSerialPortTypeNames(const AsioSerialPortTypeNames& cc);
    AsioSerialPortTypeNames();

    static std::map<unsigned int, std::string> flow_control_values_;
    static std::map<unsigned int, std::string> stop_bit_values_;
    static std::map<unsigned int, std::string> parity_values_;

    struct MapValue: public std::binary_function<
        std::pair<unsigned int, std::string>, std::string, bool >
    {
      bool operator () ( const std::pair<unsigned int, std::string> p, std::string str ) const
      {
        return(p.second == str);
      }
    };
};


//-----------------------------------------------------------------------------


class SerialPortBase
{
  protected:
//    SerialPortBase(boost::asio::io_service& io, XMLParser& parser);
    SerialPortBase(boost::asio::io_service& io);
    virtual ~SerialPortBase();

    void setPortName(const std::string& name);
    void open();
    void close();
    void setBaudRate(const unsigned int rate);
    void setFlowControl(const std::string& type);
    void setParity(const std::string& type);
    void setStopBits(const std::string& bits);
    void setCharacterSize(const unsigned int size);

    template<typename T> void sync_read(std::vector<T>& values);
    template<typename T> void sync_read(std::vector<T>& values, unsigned int bytes_to_receive);

    template<typename T> void async_read(std::vector<T>& values);
    template<typename T> void async_read(std::vector<T>& values, unsigned int bytes_to_receive);

    template<typename T> void sync_write(std::vector<T>& values);
    template<typename T> void async_write(std::vector<T>& values);

    std::string getSerialPortName()
    {
      return(port_name_);
    }

    bool isDataAvailable()
    {
      return(data_available_);
    }

    bool wasDataWritten()
    {
      return(data_written_);
    }

  private:
    void handleAsyncRead(const boost::system::error_code& error,
                    std::size_t bytes_transferred );
    void handleAsyncWrite(const boost::system::error_code& error,
                    std::size_t bytes_transferred );

  private:
    boost::asio::serial_port    serial_port_;
    std::string                 port_name_;

    bool                        data_available_;
    bool                        data_written_;

    unsigned int baud_rate_;
    unsigned int flow_control_type_;
    unsigned int parity_;
    unsigned int stop_bits_;
    unsigned int character_size_;

    AsioSerialPortTypeNames& asio_types_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// template code:

template<typename T> void SerialPortBase::sync_read(std::vector<T>& values)
{
  boost::system::error_code ec;
  boost::asio::read(serial_port_, boost::asio::buffer(values),
                    boost::asio::transfer_all(), ec);

  if(ec)
    throw(std::runtime_error("SerialPortBase::sync_read() -- \
                             Error reading serial port"));
}

//-----------------------------------------------------------------------------

template<typename T> void SerialPortBase::sync_read(std::vector<T>& values, unsigned int bytes_to_receive)
{
  boost::system::error_code ec;

  boost::asio::read(serial_port_, boost::asio::buffer(values),
                    boost::asio::transfer_at_least(bytes_to_receive), ec);

  if(ec)
    throw(std::runtime_error("SerialPortBase::sync_read() -- \
                             Error reading serial port -- bytes: " + bytes_to_receive));
}

//-----------------------------------------------------------------------------

template<typename T> void SerialPortBase::async_read(std::vector<T>& values)
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

template<typename T> void SerialPortBase::async_read(std::vector<T>& values, unsigned int bytes_to_receive)
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

template<typename T> void SerialPortBase::sync_write(std::vector<T>& values)
{
  boost::asio::write(serial_port_, boost::asio::buffer(values));
}

//-----------------------------------------------------------------------------

template<typename T> void SerialPortBase::async_write(std::vector<T>& values)
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
//-----------------------------------------------------------------------------

}  //tobiss

#endif // SERIALPORTBASE_H
