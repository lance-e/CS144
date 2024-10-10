#include "tcp_sender.hh"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
    return seqno_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
    return consecutive_retransmission_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
    TCPSenderMessage msg;
    if (fin_) return;
    if (!syn_) {
        msg.seqno = Wrap32::wrap(0, isn_);
        msg.SYN = true;
        if (reader().is_finished()) msg.FIN = true;
        msg.RST = writer().has_error();

        wait_ack_.push(msg);
        next_seqno_ += msg.sequence_length();
        seqno_in_flight_ += msg.sequence_length();

        transmit(move(msg));

        syn_ = true;
        if (!retransmission_timer_running){
            retransmission_timer_running = true;
            time_ = 0 ;
        }
        return;
    }

    window_size_ = window_size_ > 0 ? window_size_ : 1;

    //fin 
    if (reader().is_finished() && next_seqno_ < receive_seqno_ + window_size_){
        msg.seqno = Wrap32::wrap(next_seqno_, isn_),
        msg.FIN = true,

        wait_ack_.push(msg);
        next_seqno_ += msg.sequence_length();
        seqno_in_flight_ += msg.sequence_length();

        transmit(move(msg));

        if (!retransmission_timer_running){
            retransmission_timer_running = true;
            time_ = 0 ;
        }
        fin_ = true;
        return;
    }

    // 
    while(reader().bytes_buffered() && next_seqno_ < receive_seqno_ + window_size_){
        size_t len = min(TCPConfig::MAX_PAYLOAD_SIZE ,static_cast<size_t>( window_size_ + receive_seqno_ - next_seqno_));
        size_t payload_len = min(len , reader().bytes_buffered());
        string bytes{};
        string_view byte_view{};
        while (payload_len){
            byte_view = input_.reader().peek();
            if (payload_len < byte_view.size() ){
                bytes.append(byte_view.substr(0 , payload_len));
                input_.reader().pop(payload_len);
                break;
            }
            bytes.append(byte_view);
            input_.reader().pop(byte_view.size());
            payload_len -= byte_view.size();
        }

        // create segment
        msg.seqno = Wrap32::wrap(next_seqno_, isn_),
        msg.payload = move(bytes),
        msg.FIN = (input_.reader().is_finished() && msg.sequence_length() < window_size_) ? true : false;

        wait_ack_.push(msg);

        next_seqno_ += msg.sequence_length();
        seqno_in_flight_ += msg.sequence_length();

        transmit(move(msg));

        if (!retransmission_timer_running){
            retransmission_timer_running = true;
            time_ = 0 ;
        }

    }


}

TCPSenderMessage TCPSender::make_empty_message() const
{
    return {.seqno = Wrap32::wrap(next_seqno_ , isn_) };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
    uint64_t ackno_ = msg.ackno->unwrap(isn_, next_seqno_) ;
    if (ackno_ > next_seqno_) return;
    if (ackno_ >= receive_seqno_) {
        receive_seqno_ = ackno_;
        window_size_ = msg.window_size;
    }
    
    // bool acked = false;
    while(!wait_ack_.empty()){
        if (ackno_ < wait_ack_.front().seqno.unwrap(isn_, next_seqno_) + wait_ack_.front().sequence_length()){
            return;
        }
        seqno_in_flight_ -= wait_ack_.front().sequence_length();
        wait_ack_.pop();
        // acked = true;
        timeout_ = initial_RTO_ms_;
        consecutive_retransmission_ = 0;
        time_ = 0 ;
    }

    if (fin_ && wait_ack_.empty()) seqno_in_flight_ = 0 ;
    retransmission_timer_running = !wait_ack_.empty();

}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
    if (!retransmission_timer_running) return;

    time_ += ms_since_last_tick;

    if (time_ >= timeout_ && !wait_ack_.empty()){
        TCPSenderMessage sendermsg = wait_ack_.front();
        transmit(sendermsg); // Retransmission segment
        
        if (window_size_ > 0 || sendermsg.SYN){
            consecutive_retransmission_ ++;
            timeout_ *= 2;
        }

        time_ = 0 ;  //reset to 0
        retransmission_timer_running = true;
    }
}
