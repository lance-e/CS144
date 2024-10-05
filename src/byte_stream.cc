#include "byte_stream.hh"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <queue>
#include <string>
#include <string_view>
#include <utility>

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity )
  , stream_(queue<string>())
  , stream_len_(0)
  , front_view_()
  , pushed_bytes_( 0 )
  , poped_bytes_( 0 )
  , closed_( false )
  , error_( false )
{}

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::push( string data )
{
  size_t allow_len { 0 };
  allow_len = min( data.size(), capacity_ - stream_len_ );
  if (!allow_len) return ;
  data = data.substr( 0, allow_len );

  stream_.push(move(data));
  if(stream_.size() == 1) front_view_ = stream_.front();
  stream_len_ += allow_len;
  pushed_bytes_ += allow_len;
}

void Writer::close()
{
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - stream_len_;
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_bytes_;
}

bool Reader::is_finished() const
{
  return closed_ && !stream_len_;
}

uint64_t Reader::bytes_popped() const
{
  return poped_bytes_;
}

string_view Reader::peek() const
{
    return front_view_;
}

void Reader::pop( uint64_t len )
{
  len = min( stream_len_, len );
  size_t size = len;
  while(!stream_.empty() && size > 0 ){
      uint64_t front_size = front_view_.size();

      if (size >= front_size){
          stream_.pop();
          if (!stream_.empty()) front_view_ = stream_.front();
          else front_view_ = "";
          size -= front_size;
      }else{
          front_view_.remove_prefix(size);
          size = 0 ;
      }
  }
  stream_len_ -= len;
  poped_bytes_ += len;
}

uint64_t Reader::bytes_buffered() const
{
    return stream_len_;
}
