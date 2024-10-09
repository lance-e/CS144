#include "tcp_receiver.hh"
#include "tcp_receiver_message.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
    if (message.RST) {
        reader().set_error();
        return;
    }
    /* if (message.SYN) { */
        /* isn_ = move(message.seqno); */
    /* } */
    /* if (isn_ == Wrap32(-1)) return ; */
    /* reassembler_.insert(message.seqno.unwrap(isn_, reassembler_.writer().bytes_pushed() +1 ) + message.SYN - 1, message.payload, message.FIN); */

    
    if (message.SYN) isn_ = Wrap32(message.seqno);
    if ( isn_ ) {
    reassembler_.insert( message.seqno.unwrap( isn_.value(), reassembler_.writer().bytes_pushed() ) - !message.SYN,
                        message.payload,
                        message.FIN);
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
    /* uint16_t win =static_cast<uint16_t>( min( static_cast<uint64_t>(UINT16_MAX) , reassembler_.writer().available_capacity()) ); */
    /* TCPReceiverMessage ret {}; */
    /* ret.ackno  = isn_ == Wrap32(-1) ? nullopt : optional(isn_ + reassembler_.writer().bytes_pushed() + 1 + writer().is_closed()); */
    /* ret.window_size = win; */
    /* ret.RST = reader().has_error(); */
    /* return ret; */

auto ackno = isn_;
  if ( ackno ) {
    ackno.emplace( ackno.value() + 1 + reassembler_.writer().bytes_pushed() + reassembler_.writer().is_closed() );
  }
  return {
    .ackno = ackno,
    .window_size = static_cast<uint16_t>( min( reassembler_.writer().available_capacity(), uint64_t { UINT16_MAX } ) ),
    .RST = reader().has_error(),
  };
}
