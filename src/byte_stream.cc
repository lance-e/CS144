#include "byte_stream.hh"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <queue>
#include <string_view>

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity )
  , stream_( queue<char, deque<char>>() )
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
  allow_len = min( data.size(), capacity_ - stream_.size() );
  data = data.substr( 0, allow_len );

  for ( const char& ch : data ) {
    stream_.push( ch );
  }
  pushed_bytes_ += allow_len;
}

void Writer::close()
{
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - stream_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_bytes_;
}

bool Reader::is_finished() const
{
  return closed_ && stream_.empty();
}

uint64_t Reader::bytes_popped() const
{
  return poped_bytes_;
}

string_view Reader::peek() const
{
  return string_view { &stream_.front(), 1 };
}

void Reader::pop( uint64_t len )
{
  len = min( uint64_t( stream_.size() ), len );
  size_t size = len;
  while ( size-- ) {
    stream_.pop();
  }
  poped_bytes_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  return pushed_bytes_ - poped_bytes_;
}
