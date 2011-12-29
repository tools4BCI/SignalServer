#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace tobiss
{

/**
* @class DataBuffer
* @brief Thread safe FIFO buffer
* @tparam T Datatype to be buffered
*/
template<class T>
class DataBuffer
{
public:
  typedef boost::circular_buffer<T> container_type;
  typedef typename container_type::size_type size_type;
  typedef typename container_type::value_type value_type;
  typedef typename boost::call_traits<value_type>::param_type param_type;

  /**
    * @brief Constructor
    * @param[in] capacity DataBuffer::size_type Buffer size
    */
  explicit DataBuffer( size_type capacity ) : unread_(0), container_(capacity) { }  

  /**
    * @brief Change Buffer Size
    * @param[in] capacity DataBuffer::size_type Buffer size
    * @remark May cause reallocation.
    */
  void resize( size_type capacity )
  {
    boost::mutex::scoped_lock lock( mutex_ );
    container_.resize( capacity );
    lock.unlock( );
  }

  /**
    * @brief returns number of unread items available in the buffer.
    * @return DataBuffer::size_type Number of unread items
    */
  size_type getNumAvail(  )
  {
    boost::mutex::scoped_lock lock( mutex_ );
    return unread_;
    // the mutex is unlocked by scoped_lock's destructor.
  }

  /**
    * @brief Insert a new item (sample/frame/packet/...) in the buffer
    * @param[in] item DataBuffer::param_type Item to insert
    * @remark If the buffer is full the function blocks until another thread retrieves an item!
    */
  void insert_blocking( param_type item )
  {
    boost::mutex::scoped_lock lock( mutex_ );
    cond_not_full_.wait( lock, boost::bind( &DataBuffer<value_type>::is_not_full, this ) );
    container_.push_front( item );
    unread_++;
    lock.unlock( );
    cond_not_empty_.notify_one( );
  }

  /**
    * @brief Insert a new item (sample/frame/packet/...) in the buffer
    * @param[in] item DataBuffer::param_type Item to insert
    * @remark If the buffer is full an exception of type std::runtime_error is thrown.
    */
  void insert_throwing( param_type item )
  {
    boost::mutex::scoped_lock lock( mutex_ );
    if( is_not_full() )
    {
      container_.push_front( item );
      unread_++;
      lock.unlock( );
      cond_not_empty_.notify_one( );
    }
    else
    {
      lock.unlock( ); // This unlock() is redundant. The scoped_lock's destructor should take care of unlocking the mutex.
      throw std::runtime_error( "Attempted to insert into full DataBuffer." );
    }
  }  

  /**
    * @brief Insert a new item (sample/frame/packet/...) in the buffer
    * @param[in] item DataBuffer::param_type Item to insert
    * @remark If the buffer is full, the oldest item is dropped from the buffer
    * @warning Experimental and untested!
    */
  void insert_overwriting( param_type item )
  {
    boost::mutex::scoped_lock lock( mutex_ );
    container_.push_front( item );
    if( is_not_full() )
      unread_++;
    lock.unlock( );
    cond_not_empty_.notify_one( );
  }  

  /**
    * @brief get next item (sample/frame/packet/...) from the buffer
    * @param[out] pItem DataBuffer::value_type* Pointer to storage
    * @remark If the buffer is empty the function blocks until another thread inserts an item!
    */
  void getNext_blocking( value_type *pItem )
  {
    boost::mutex::scoped_lock lock( mutex_ );
    cond_not_empty_.wait( lock, boost::bind( &DataBuffer<value_type>::is_not_empty, this ) );
    *pItem = container_[--unread_];
    lock.unlock( );
    cond_not_full_.notify_one( );
  }  

  /**
    * @brief get next item (sample/frame/packet/...) from the buffer
    * @param[out] pItem DataBuffer::value_type* Pointer to storage
    * @remark If the buffer is empty an exception of type std::runtime_error is thrown.
    */
  void getNext_throwing( value_type *pItem )
  {
    boost::mutex::scoped_lock lock( mutex_ );
    if( is_not_empty() )
    {
      *pItem = container_[--unread_];
      lock.unlock( );
      cond_not_full_.notify_one( );
    }
    else
    {
      lock.unlock( ); // This unlock() is redundant. The scoped_lock's destructor should take care of unlocking the mutex.
      throw std::runtime_error( "Attempted to read from empty DataBuffer." );
    }
  }

private:
  DataBuffer( const DataBuffer& );            // No Copy Constructor!
  DataBuffer &operator=( const DataBuffer& ); // No Assignment Operator!

  bool is_not_empty( ) const { return unread_ > 0; }
  bool is_not_full( ) const { return unread_ < container_.capacity( ); }

  size_type unread_;
  container_type container_;

  boost::mutex mutex_;
  boost::condition_variable cond_not_empty_;
  boost::condition_variable cond_not_full_;
};

}

#endif
