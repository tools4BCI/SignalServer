#ifndef SERIALPORTBASE_H
#define SERIALPORTBASE_H

#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "hw_thread.h"

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
        pair<unsigned int, std::string>, std::string, bool >
    {
      bool operator () ( const pair<unsigned int, std::string> p, std::string str ) const
      {
        return(p.second == str);
      }
    };
};


//-----------------------------------------------------------------------------


class SerialPortBase : public HWThread
{
  protected:
    SerialPortBase(boost::asio::io_service& io, XMLParser& parser);
    virtual ~SerialPortBase();

    void open(const std::string port_name);
    void close();
    void setBaudRate(const unsigned int rate);
    void setFlowControl(const std::string& type);
    void setParity(const std::string& type);
    void setStopBits(const std::string& bits);
    void setCharacterSize(const unsigned int size);

    void sync_read(std::vector<char>& values);
    void sync_read(std::vector<char>& values, unsigned int bytes_to_receive);

    void async_read(std::vector<char>& values);
    void async_read(std::vector<char>& values, unsigned int bytes_to_receive);

    void sync_write(std::vector<char>& values);
    void async_write(std::vector<char>& values);

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

}  //tobiss


#endif // SERIALPORTBASE_H
