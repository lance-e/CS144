#include "reassembler.hh"
#include <algorithm>
#include <bits/utility.h>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <utility>

using namespace std;

void Reassembler::insert( uint64_t first_index_, string data, bool is_last_substring )
{
    if (output_.writer().is_closed()) return ;
    if (data.empty()){
        if (is_last_substring){
            output_.writer().close();
        }
        return;
    }
    
    //record the final index 
    if (is_last_substring) {
        last_string_index_ = first_index_ + data.size();
    }
    // all overlapping
    if (first_index_ + data.size() <= output_.writer().bytes_pushed()){
        return ;
    }

    // part overlapping
    if (first_index_ < output_.writer().bytes_pushed()){
        string sub = data.substr(output_.writer().bytes_pushed() - first_index_);
        output_.writer().push(move(sub));
        poped_byte_ = writer().bytes_pushed();
        if (is_last_substring){
            if (first_index_ + data.size() == output_.writer().bytes_pushed()){
                cathe_.clear();
                output_.writer().close();
            }
        }else{
            push_stored_bytes(first_index_ + data.size());
        }
        return;
    }

    if (!output_.writer().available_capacity()) return;

    if (first_index_ ==output_.writer().bytes_pushed() ){
        output_.writer().push(move(data));
        poped_byte_ = writer().bytes_pushed();
        push_stored_bytes(first_index_ + data.size());
        return;
    }

    if (first_index_  >= output_.writer().bytes_pushed() + output_.writer().available_capacity()){
        return;
    }

    if (cathe_.contains(first_index_) && cathe_.at(first_index_).size() >= data.size()){
        return;
    }


    if (first_index_ + data.size() - output_.writer().bytes_pushed() <= output_.writer().available_capacity()){
        cathe_[first_index_] = move(data);
    }else {
        cathe_[first_index_] = data.substr(0 , output_.writer().bytes_pushed()  + output_.writer().available_capacity() - first_index_);
    }
}

uint64_t Reassembler::bytes_pending() const
{
    size_t ans{};
    size_t temp = poped_byte_;
    for (const auto& it : cathe_){
        if (it.first >= temp){
            ans += it.second.size();
            temp = it.first + it.second.size();
        }else {
            if (it.first + it.second.size() > temp){
                ans += it.first + it.second.size() - temp;
            }
            temp = max(temp , it.first + it.second.size());
        }
    }
    return ans;
}

void Reassembler::push_stored_bytes(size_t index_){
    if (index_ > output_.writer().bytes_pushed()) return;
    size_t prev = poped_byte_ ;
    for (const auto & it : cathe_){
        if (it.first > output_.writer().bytes_pushed()) break;
        if (it.first >=  prev){
            prev += it.second.size();
            output_.writer().push(move(it.second));
        }else {
            if (it.first + it.second.size() > prev){
                output_.writer().push(it.second.substr(prev - it.first));
            }
            prev = max(prev , it.first + it.second.size());
        }
    }
    poped_byte_ = writer().bytes_pushed();
    if (output_.writer().bytes_pushed() == last_string_index_ ){
        cathe_.clear();
        output_.writer().close();
    }
}
