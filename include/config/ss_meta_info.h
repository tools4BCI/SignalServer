
#ifndef SERVER_META_INFO_H
#define SERVER_META_INFO_H

#include <boost/cstdint.hpp>

// STL
#include <string>
#include <map>
#include <vector>

#include <boost/cstdint.hpp>

namespace tobiss
{

//-----------------------------------------------------------------------------

class SubjectInfo
{
  public:
    enum Sex {
      Male   = 1,
      Female = 2
    };

    enum Handedness {
      RightHanded = 1,
      LeftHanded = 2
    };

    enum ShortInfoType{
      Glasses = 1,
      Smoking
    };

    enum ShortInfoValue {
      Unknown = 0,
      No,
      Yes
    };

    typedef std::map<ShortInfoType, ShortInfoValue> ShortInfoMap;

  public:
    std::string id() const { return id_; }
    void setId(const std::string& id) { id_ = id; }

    std::string firstName() const { return firstname_; }
    void setFirstName(const std::string& name) { firstname_ = name; }

    std::string surname() const { return surname_; }
    void setSurname(const std::string& name) { surname_ = name; }

    std::string birthday() const { return birthday_; }
    void setBirthday(const std::string& birthday) { birthday_ = birthday; }

    int sex() const { return sex_; }
    void setSex(int sex) { sex_ = sex; }

    int handedness() const { return handedness_; }
    void setHandedness(int handedness) { handedness_ = handedness; }

    void setMedication(std::string& medication) { medication_ =  medication; }
    std::string medication() const { return medication_; }

    void setShortInfo(ShortInfoType info, ShortInfoValue value)
    {
      short_infos_[info] = value;
    }

    ShortInfoValue shortInfo(ShortInfoType info) const
    {
      ShortInfoMap::const_iterator it = short_infos_.find(info);
      if (it == short_infos_.end()) return Unknown;
      return (*it).second;
    }

    ShortInfoMap& shortInfoMap() { return short_infos_; }

    const ShortInfoMap& shortInfoMap() const { return short_infos_; }

  private:
    std::string  id_;
    std::string  firstname_;
    std::string  surname_;
    std::string  birthday_;
    std::string  medication_;
    int          handedness_;
    int          sex_;
    ShortInfoMap short_infos_;
};

//-----------------------------------------------------------------------------

class Channel
{
  public:
    void setId(const std::string& id) { id_ = id; }
    std::string id() const { return id_; }

    void setDeviceId(const boost::uint32_t& id) { device_id_ = id; }
    boost::uint32_t deviceId() const { return device_id_; }

    void setDescription(const std::string& id) { description_ = id; }
    std::string description() const { return description_; }

    void setPhysicalRange(const int& range) { physical_range_ = range; }
    int physicalRange() const { return physical_range_; }

    void setDigitalRange(const int& range) { digital_range_ = range; }
    int digitalRange() const { return digital_range_; }

    void setDataType(int type) { data_type_ = type; }
    int dataType() const { return data_type_; }

    void setBpFilter(const std::pair<float, float> bp_filter) { bandpass_filter_ = bp_filter; }
    std::pair<float, float> bpFilter() const { return bandpass_filter_; }

    void setNFilter(const std::pair<float, float> n_filter) { notch_filter_ = n_filter; }
    std::pair<float, float> nFilter() const { return notch_filter_; }

  private:
    std::string               id_;
    boost::uint32_t           device_id_;
    std::string               description_;
    int                       physical_range_;
    int                       digital_range_;
    std::pair<float, float>   bandpass_filter_;
    std::pair<float, float>   notch_filter_;
    int                       data_type_;

};

//-----------------------------------------------------------------------------

class Signal
{
  public:
      void setType(const std::string& type) { type_ = type; }

      std::string type() const { return type_; }

      boost::uint16_t blockSize() const { return block_size_; }

      void setBlockSize(boost::uint16_t block_size) { block_size_ = block_size; }

      boost::uint16_t samplingRate() const { return sampling_rate_; }

      void setSamplingRate(boost::uint16_t sampling_rate) { sampling_rate_ = sampling_rate; }

      const std::vector<Channel>& channels() const { return channels_; }

      std::vector<Channel>& channels() { return channels_; }

  private:
    std::string           type_;
    boost::uint16_t       block_size_;
    boost::uint16_t       sampling_rate_;
    std::vector<Channel>  channels_;
};

//-----------------------------------------------------------------------------

class SignalInfo
{
  public:
    typedef std::map<std::string, Signal> SignalMap;

  public:
    boost::uint16_t masterSamplingRate() const { return master_sampling_rate_; }

    void setMasterSamplingRate(boost::uint16_t rate) { master_sampling_rate_ = rate; }

    boost::uint16_t masterBlockSize() const { return master_block_size_; }

    void setMasterBlockSize(boost::uint16_t size) { master_block_size_ = size; }

    const SignalMap& signals() const { return signals_; }

    SignalMap& signals() { return signals_; }

  private:
    boost::uint16_t  master_block_size_;
    boost::uint16_t  master_sampling_rate_;
    SignalMap signals_;
};

class HardwareInfo
{
  public:
    enum OperatingMode {
      Master = 0,
      Slave,
      Aperiodic
    };

  public:
    std::string hwName() const { return hw_name_; }
    void setHwName(const std::string& name) { hw_name_ = name; }

    std::string serial() const { return serial_; }
    void setSerial(const std::string& serial) { serial_ = serial; }

    boost::uint16_t numberOfChannels() const { return number_of_channels_; }
    void setNumberOfChannels(boost::uint16_t number) { number_of_channels_ = number; }

    int physicalRange() const { return physical_range_; }
    void setPhysicalRange(int number) { physical_range_ = number; }

    boost::uint16_t dataType() const { return data_type_; }
    void setDataType(boost::uint16_t number) { data_type_ = number; }

    boost::uint16_t samplingRate() const { return sampling_rate_; }
    void setSamplingRate(boost::uint16_t number) { sampling_rate_ = number; }

    OperatingMode operatingMode() const { return operating_mode_; }
    void setOperatingMode(OperatingMode mode) { operating_mode_ = mode; }

  private:
    std::string       hw_name_;
    std::string       serial_;
    boost::uint16_t   number_of_channels_;
    int               physical_range_;
    boost::uint16_t   data_type_;
    boost::uint16_t   sampling_rate_;
    OperatingMode     operating_mode_;
};

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // UDPSERVER_H
