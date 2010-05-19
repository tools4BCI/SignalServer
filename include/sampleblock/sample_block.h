
/**
* @file sample_block.h
*
* @brief sample_block is designed to store and sort samples directly acquired from hardware devices.
*
* A SampleBlock stores samples, directly acquired from hardware devices in ascending order to
* their signal types. It can store blocked data directly by giving a already block oriented
* sample vector or by appending block by block to a SampleBlock.
* Blocksize and number of channels to store in a SampleBlock have to be
**/

#ifndef SAMPLEBLOCK_H
#define SAMPLEBLOCK_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#include <boost/cstdint.hpp>

using namespace std;

//using boost::uint16_t;
//using boost::uint32_t;
//using boost::uint64_t;

//-----------------------------------------------------------------------------
/**
* @class SampleBlock
*
* @brief  A container storing and sorting samples directly acquired from a data acquisition device.
* @tparam T Datatype to be used to store samples in this class.
*
* A SampleBlock stores samples directly acquired from a data acquisition device and sorts them
* internally in the correct ascending order (by their signal type -- EEG first, then EMG, ...
* see defines.h for signal types and their order).
* The sorting is done by a simple bubble-sort algorithm.
* A SampleBlock can handle blocked and non-blocked data.
* Non-blocked data is stored as one channel in an element of a vector with first the channels of one
* signal type ascending order and afterwards the signal types also in ascending order.
*  --> e.g. EEG_ch1 EEG_ch2 EEG_ch4   EMG_ch3 EMG_ch6   ....
*   (if signal types are not conected ascending to the hardware device, the sample block will
*    reorder the channels like demostrated above  -- an EMG channel was placed "between" EEG
*    channels and channel 5 was left blank)
* Using a blocked data acquisition samples are stored that way:
*  --> e.g. EEG_ch1_bl1 EEG_ch1_bl2  EEG_ch2_bl1 EEG_ch2_bl2  EEG_ch4_bl1 EEG_ch4_bl2
*           EMG_ch3_bl1 EMG_ch3_bl2  EMG_ch6_bl1 EMG_ch6_bl2
* Blocked data storage can be achieved by sampling without blocks and appending samples to the
* SampleBlock (appendBlock()) or by inserting a samples vector containing already blocked data.
*
* The number of channels and the blocksize have to be defined by initializing the SampleBlock.
* @warning Inserting more or less samples than defined through channels and blocks is not possible!
*/
template<class T> class SampleBlock
{
  public:
    /**
    * @brief Default constructor -- building an empty SampleBlock (unusable before init()).
    */
    SampleBlock()
      : blocks_ (0), channels_(0), homogenous_(1),curr_block_(0) { }

    /**
    * @brief Default Destructor
    */
    virtual ~SampleBlock()    {  }

    /**
    * @brief Return the blocksize, the data can be stored with.
    */
    boost::uint16_t getNrOfBlocks()    {  return(blocks_ );  }
    /**
    * @brief Return the number of channels that can be stored in the SampleBlock.
    */
    boost::uint16_t getNrOfChannels()    {  return(channels_);  }
    /**
    * @brief Return the amount of samples stored in the SampleBlock.
    */
    boost::uint16_t getNrOfSamples()    {  return(samples_.size());  }
    /**
    * @brief Return the number of different signal types that can be stored in the SampleBlock.
    */
    boost::uint16_t getNrOfSignalTypes()     { return(block_info_.size());  }
    /**
    * @brief Delete all samples stored in the SampleBlock.
    */
    void deleteSamples()    {  samples_.clear();  }
    /**
    * @brief Sets the block counter for blocks to be appended bach to zero.
    */
    void reset()     { curr_block_ = 0;  }

    /**
    * @brief Access the n-th sample.
    * @throw std::out_of_range
    */
    T at(boost::uint32_t n)    {  return(samples_.at(n));  }
    /**
    * @brief Access the n-th sample.
    */
    T operator[](boost::uint32_t n)    {  return(samples_[n]);  }

    //-------------------------------------------

    /**
    * @brief Sets the block counter for blocks to be appended bach to zero.
    * @param[in] blocksize The blocksize, the data will be stored.
    * @param[in] nr_ch The number of channels to store.
    * @param[in] sig_types A vector specifying the signal type of every sampled channel.
    *
    * init is used to initialize a SampleBlock. After re-initialization, inserting more
    * or less channels or blocks into a SampleBlock is not possible and will cause an exception
    * if data is read to early (using appending blocks) or if a too short or long vector is
    * inserted.
    */
    void init(boost::uint16_t blocksize, boost::uint16_t nr_ch, vector<boost::uint32_t> sig_types)
    {
      #ifdef DEBUG
        cout << "SampleBlock: init" << endl;
      #endif

	    using boost::uint16_t;
      using boost::uint32_t;

      if(nr_ch != sig_types.size())
        throw(std::logic_error("SampleBlock::init -- sig_types.size() differs from number of channels given!"));

      blocks_  = blocksize;
      channels_ = nr_ch;
      types_input_ = sig_types;

      if(nr_ch == 0)
        return;

      samples_.resize(blocks_ *channels_);

      uint32_t o_tmp;
      for(unsigned int i=0; i < sig_types.size(); i++)
        for(unsigned int x=0; x < sig_types.size()-1-i; x++)
          if(sig_types[x] > sig_types[x+1])
          {
            o_tmp = sig_types[x];
            sig_types[x] =sig_types[x+1];
            sig_types[x+1] = o_tmp;
          }
      //       types_ordered_ = sig_types;

      pair<uint16_t, uint16_t> pi;   //  ... pos. information (offset, nr_values)
      pair<uint32_t, pair<uint16_t, uint16_t> > si; // samples info (flag)
      uint16_t pos = 0;
      for(unsigned int n = 1; n < sig_types.size(); n++)
        if(sig_types[n-1] != sig_types[n])
        {
          homogenous_ = 0;
          pi = make_pair(pos*blocks_ ,(n-pos)*blocks_ );   // (n - pos)
          si = make_pair(sig_types[n-1], pi);
          block_info_.insert(si);
          pos = n;
        }
      pi = make_pair(pos*blocks_ , (channels_-pos)*blocks_ );
      si = make_pair(sig_types[pos], pi);
      block_info_.insert(si);
    }
    //-------------------------------------------
    /**
    * @brief Get a vector with the signal types to be stored in the SampleBlock.
    * @return A vector with the signal types (order like stored in the SampleBlock).
    */
	vector<boost::uint32_t> getTypes()
    {
      #ifdef DEBUG
        cout << "SampleBlock: getTypes" << endl;
      #endif

	    using boost::uint16_t;
      using boost::uint32_t;

      vector<uint32_t> v;
      for(map<uint32_t, pair<uint16_t, uint16_t> >::iterator it = block_info_.begin();
          it != block_info_.end(); it++)
        v.push_back(it->first);
      return(v);
    }
    //-------------------------------------------
    /**
    * @brief Get a vector with all samples stored.
    * @return A vector containing all samples.
    * @throw  std::length_error if the samples_ vector size does not equal blocks_ *channels_
    */
    vector<T> getSamples()
    {
      #ifdef DEBUG
        cout << "SampleBlock: getSamples" << endl;
      #endif

      checkBlockIntegrity();
      return(samples_);
    }
    //-------------------------------------------
    /**
    * @brief Get the flag of the n-th signal type.
    * @return The flag.
    * @throw  std::invalid_argument if n bigger than the number of signal types stored.
    */
    boost::uint32_t getFlagByNr(boost::uint32_t nr)
    {
      #ifdef DEBUG
        cout << "SampleBlock: getFlagByNr" << endl;
      #endif

	    using boost::uint16_t;
      using boost::uint32_t;

      if(nr > block_info_.size())
        throw(std::invalid_argument("getFlagByNr -- Signal number higher than nr. of different signals available!"));

      map<uint32_t, pair<uint16_t, uint16_t> >::iterator m_it(block_info_.begin());
      for(uint32_t n = 0; n < nr; n++)
        m_it++;

      return(m_it->first);
    }
    //-------------------------------------------
    /**
    * @brief Get a vector containing samples defined by a flag.
    * @param[in] flag The Flag specifying the signal
    * @return A vector containing wanted samples.
    * @throw  std::invalid_argument if flag is not defined set in the SampleBlock.
    */
	vector<T> getSignalByFlag(boost::uint32_t flag)
    {
      #ifdef DEBUG
        cout << "SampleBlock: getSignalByFlag" << endl;
      #endif
	  
	    using boost::uint16_t;
      using boost::uint32_t;

      map<uint32_t, pair<uint16_t, uint16_t> >::iterator m_it(block_info_.find(flag));
      if(m_it != block_info_.end())
      {
        checkBlockIntegrity();
        typename vector<T>::iterator v_it(samples_.begin() + m_it->second.first);
        vector<T> v(v_it, v_it + m_it->second.second);
        return(v);
      }
      else
        throw(std::invalid_argument("getSinalByFlag -- FLAG not set in SampleBlock!"));
    }
    //-------------------------------------------
    /**
    * @brief Get a vector containing samples defined by a flag.
    * @param[in] flag The Flag specifying the signal
    * @param[out] v A vector to insert desired samples.
    * @throw  std::invalid_argument if flag is not defined set in the SampleBlock.
    */
  void getSignalByFlag(boost::uint32_t flag, vector<T>& v)
    {
      #ifdef DEBUG
        cout << "SampleBlock: getSignalByFlag" << endl;
      #endif

	    using boost::uint16_t;
      using boost::uint32_t;

      map<uint32_t, pair<uint16_t, uint16_t> >::iterator m_it(block_info_.find(flag));
      if(m_it != block_info_.end())
      {
        checkBlockIntegrity();
        typename vector<T>::iterator v_it(samples_.begin() + m_it->second.first);

        v.assign(v_it, v_it + m_it->second.second);
      }
      else
        throw(std::invalid_argument("getSinalByFlag -- FLAG not set in SampleBlock!"));
    }
    //-------------------------------------------
    /**
    * @brief Get samples of the n-th signal type stored.
    * @param[in] nr specifying the number of the signal in the SampleBlock (first, second, ...)
    * @return A vector containing desired samples.
    * @throw  std::invalid_argument if nr is higher than signals stord.
    */
	vector<T> getSignalByNr(boost::uint32_t nr)
    {
      #ifdef DEBUG
        cout << "SampleBlock: getSignalByNr" << endl;
      #endif

	    using boost::uint16_t;
      using boost::uint32_t;

      if(nr > block_info_.size())
        throw(std::invalid_argument("getSinalByNr -- Signal number higher than nr. \
          of different signals available!"));
      checkBlockIntegrity();

      map<uint32_t, pair<uint16_t, uint16_t> >::iterator m_it(block_info_.begin());
      for(uint32_t n = 0; n < nr; n++)
        m_it++;
      typename vector<T>::iterator v_it(samples_.begin() + m_it->second.first);
      vector<T> v(v_it, v_it + m_it->second.second);
      return(v);
    }
    //-------------------------------------------
    /**
    * @brief Get samples of the n-th signal type stored.
    * @param[in] nr specifying the number of the signal in the SampleBlock (first, second, ...)
    * @param[out] v A vector to insert desired samples.
    * @throw  std::invalid_argument if nr is higher than signals stord.
    */
	void getSignalByNr(boost::uint32_t nr, vector<T>& v)
    {
      #ifdef DEBUG
        cout << "SampleBlock: getSignalByNr" << endl;
      #endif

	    using boost::uint16_t;
      using boost::uint32_t;

      if(nr > block_info_.size())
        throw(std::invalid_argument("getSinalByNr -- Signal number higher than nr. \
        of different signals available!"));
      checkBlockIntegrity();

      map<uint32_t, pair<uint16_t, uint16_t> >::iterator m_it(block_info_.begin());
      for(uint32_t n = 0; n < nr; n++)
        m_it++;
      typename vector<T>::iterator v_it(samples_.begin() + m_it->second.first);

      v.assign(v_it, v_it + m_it->second.second);
    }
    //-------------------------------------------
    /**
    * @brief Append samples to the SampleBlock. (used to build blocked data)
    * @param[in] v A vector containing the samples to be appended. (vector must not contain already blocked data!)
    * @throw  std::length_error if v.size does not equal channels or trying to append more blocks than defined.
    * @todo If needed: Possibility to append already blocked data.
    */
    void appendBlock(vector<T> v)
    {
      #ifdef DEBUG
        cout << "SampleBlock: appendBlock" << endl;
      #endif

      if(v.size() != channels_)
        throw(std::length_error("appendBlock: Appended Block contains more values than sampled channels!"));

      if(blocks_  < curr_block_)
        throw(std::length_error("appendBlock: Tried to append more blocks than specified!"));

      sort(v,types_input_);

      for(unsigned int n = 0; n < channels_; n++)
        samples_[ (n*blocks_ ) + curr_block_ ] = v[n];

      curr_block_++;
    }
    //-------------------------------------------
    /**
    * @brief Directly set samples into the SampleBlock.
    * @param[in] v The vector containing the samples to be set.
    * @throw  std::length_error if the samples vector size does not equal blocks_ *channels_
    * @todo Check, if setting already blocked data in an unordered manner performs correct sorting!!
    */
    void setSamples(vector<T> v)
    {
      #ifdef DEBUG
        cout << "SampleBlock: setSamples" << endl;
      #endif

      if(v.size() != static_cast<boost::uint32_t>(blocks_ *channels_))
      {
        string ex_str;
        string nr_samples(boost::lexical_cast<string>(v.size()));
        string available(boost::lexical_cast<string>(blocks_ *channels_));
        ex_str = "Values in vector: " + nr_samples + " -- awaiting: " + available;
          throw(std::length_error("Sample Block contains a different amount of values than sampled channels! -- " +ex_str ));
      }
      if(homogenous_)
        samples_.assign(v.begin(),v.end());
      else
      {
        sort(v,types_input_);
        samples_.assign(v.begin(),v.end());
      }
    }

//-------------------------------------

  private:
    /**
    * @brief Sort a given sample vector by an given order (unordered) vector.
    * @param[in] v A vector containing samples to be ordered.
    * @param[in] order A vector containing indices to order (must be in the same order like v!).
    *
    * Sort sorts two vectors v and order, whereby values stored in order (has to be unordered)
    * are used to order both v and order.
    * Bubble-sort is used as a sorting algorithm.
    */
	void sort(vector<T>& v, vector<boost::uint32_t> order)
    {
      #ifdef DEBUG
        cout << "SampleBlock: sort" << endl;
      #endif

      double v_tmp;
	  boost::uint32_t o_tmp;

      for(unsigned int i=0; i < order.size(); i++)
        for(unsigned int x=0; x < order.size()-1-i; x++)
          if(order[x] > order[x+1])
          {
            o_tmp = order[x];
            order[x] =order[x+1];
            order[x+1] = o_tmp;

            v_tmp = v[x];
            v[x] = v[x+1];
            v[x+1] = v_tmp;
          }
    }
    //-------------------------------------------
    /**
    * @brief Check the block integrity (if number of samples equals equals blocks_ *channels_).
    * @throw  std::length_error if the samples vector size does not equal blocks_ *channels_
    */
    void checkBlockIntegrity()
    {
      #ifdef DEBUG
        cout << "SampleBlock: checkBlockIntegrity" << endl;
      #endif

      if(samples_.size() != static_cast<boost::uint32_t>(channels_*blocks_ ))
      {
        string ex_str;
        string n(boost::lexical_cast<string>(samples_.size()));
        string c(boost::lexical_cast<string>(channels_));
        string b(boost::lexical_cast<string>(blocks_ ));
        ex_str = "("+n+ ") different to channels_ ("+c+ ") * blocks_  ("+b+ ")!";
        throw(std::length_error("Inconsistent sample block -- nr of samples" + ex_str));
      }
    }

//-----------------------------------------------

  private:
    boost::uint16_t blocks_ ;    ///< Blocksize to be used
    boost::uint16_t channels_;  ///< Number of channels to be stored
    bool homogenous_;    ///< To avoid sorting, if only one signal type is stored in the SampleBlock.
    boost::uint16_t curr_block_;    ///< Counter to know the current block.

    //vector<uint32_t> types_ordered_;   //< Signal types in an ordered manner (and how samples are stored)
    vector<boost::uint32_t> types_input_;   ///< Order, how signal types come from the hardware device.
    /**
    * @brief A Map to store every the offset and number of values for every signal type.
    *
    * key: signaltype
    * data:  pair->first ... offset    pair->second ... nr of values
    * This map is also used to compute the number of different signal types stored.
    */
    map<boost::uint32_t, pair<boost::uint16_t, boost::uint16_t> > block_info_;  // flag ... identifier, offset & nr_values as data
    vector<T> samples_;    ///<  A vector containing the data.
};

//-----------------------------------------------------------------------------


#endif // SAMPLEBLOCK_H
