/*
    This file is part of the TOBI Interface A (TiA) library.

    Commercial Usage
    Licensees holding valid Graz University of Technology Commercial
    licenses may use this file in accordance with the Graz University
    of Technology Commercial License Agreement provided with the
    Software or, alternatively, in accordance with the terms contained in
    a written agreement between you and Graz University of Technology.

    --------------------------------------------------

    GNU Lesser General Public License Usage
    Alternatively, this file may be used under the terms of the GNU Lesser
    General Public License version 3.0 as published by the Free Software
    Foundation and appearing in the file lgpl.txt included in the
    packaging of this file.  Please review the following information to
    ensure the GNU General Public License version 3.0 requirements will be
    met: http://www.gnu.org/copyleft/lgpl.html.

    In case of GNU Lesser General Public License Usage ,the TiA library
    is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with the TiA library. If not, see <http://www.gnu.org/licenses/>.

    Copyright 2010 Graz University of Technology
    Contact: TiA@tobi-project.org
*/

#ifndef CUSTOM_SIGNAL_INFO_H
#define CUSTOM_SIGNAL_INFO_H

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

// STL
#include <string>
#include <map>
#include <vector>

namespace tia
{

//-----------------------------------------------------------------------------

/**
 * @class CustomChannel
 */
class CustomChannel
{
  public:
    /**
     * @brief Get the channel name (e.g. C3).
     */
    std::string id() const { return id_; }
    /**
     * @brief Set the channel name.
     */
    void setId(const std::string& id) { id_ = id; }

    boost::uint32_t number() const { return number_; }

    void setNumber(boost::uint32_t number) { number_ = number; }

  private:
    std::string id_;    ///<
    boost::uint32_t number_;

};

//-----------------------------------------------------------------------------

/**
 * @class CustomSignal
 */
class CustomSignal
{
  public:
      /**
       * @brief Set the name of the signal type.
       */
      void setType(const std::string& type) { type_ = type; }

      /**
       * @brief Get the name of the stored signal type.
       */
      std::string type() const { return type_; }

      /**
       * @brief Get the blocksize of the stored signal type.
       */
      boost::uint16_t blockSize() const { return block_size_; }

      /**
       * @brief Set the blocksize of the stored signal type.
       */
      void setBlockSize(boost::uint16_t block_size) { block_size_ = block_size; }

      /**
       * @brief Get the sampling rate of the stored signal type.
       */
      boost::uint16_t samplingRate() const { return sampling_rate_; }

      /**
       * @brief Returns wheter the signal is apreiodic or not. Is similar to a sampling rate equal to zero.
       */
      bool isAperiodic() const { return sampling_rate_ == 0; }

      /**
       * @brief Set the sampling rate of the stored signal type.
       */
      void setSamplingRate(boost::uint16_t sampling_rate) { sampling_rate_ = sampling_rate; }

      /**
       * @brief Get the downsampling factor of the custom signal type.
       */
      boost::uint16_t DSFactor() const { return ds_factor_; }

      /**
       * @brief Set the downsampling factor for the custom signal type.
       */
      void setDSFactor(boost::uint16_t ds_factor) { ds_factor_ = ds_factor; }


      /**
       * @brief Get a vector holding specific information for every channel of this signal type.
       */
      const std::vector<CustomChannel>& channels() const { return channels_; }

      std::vector<CustomChannel>& channels() { return channels_; }

  private:
    std::string           type_;
    boost::uint16_t       block_size_;
    boost::uint16_t       sampling_rate_;
    boost::uint16_t       ds_factor_;
    std::vector<CustomChannel>  channels_;
};

//-----------------------------------------------------------------------------

/**
 * @class CustomSignalInfo
 */
class CustomSignalInfo
{
  public:
    typedef std::map<std::string, CustomSignal> CustomSignalMap;

  public:

#ifdef signals
    #define SIGNAL_INFO_HELPER signals
    #undef signals
#endif

    const CustomSignalMap& signals() const { return signals_; }

    CustomSignalMap& signals() { return signals_; }

  private:
    CustomSignalMap signals_;                       ///<

#ifdef SIGNAL_INFO_HELPER
    #define signals SIGNAL_INFO_HELPER
    #undef SIGNAL_INFO_HELPER
#endif
};

typedef boost::shared_ptr<CustomSignalInfo> CustomSignalInfoPtr;

} // Namespace tobiss

//-----------------------------------------------------------------------------

#endif // CUSTOM_SIGNAL_INFO_H
