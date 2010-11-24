/*
    This file is part of TOBI Interface A (TiA).

    TOBI Interface A (TiA) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TOBI Interface A (TiA) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TOBI Interface A (TiA).  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Christian Breitwieser
    Contact: c.breitwieser@tugraz.at
*/

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
    std::string id() const { return id_; }

    void setId(const std::string& id) { id_ = id; }

  private:
    std::string id_;
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

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // UDPSERVER_H
