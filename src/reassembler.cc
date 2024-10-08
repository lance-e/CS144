#include "reassembler.hh"
#include <algorithm>
#include <cstddef>
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
        push_stored_bytes(output_.writer().bytes_pushed());
        return;
    }

    if (first_index_ ==output_.writer().bytes_pushed() ){
        output_.writer().push(move(data));
        push_stored_bytes(output_.writer().bytes_pushed());
        return;
    }

    if (first_index_  > output_.writer().bytes_pushed() + output_.writer().available_capacity()){
        return;
    }

    if (cathe_.contains(first_index_) && cathe_.at(first_index_).size() >= data.size()){
        return;
    }


    if (first_index_ > output_.writer().bytes_pushed() ){
        if (first_index_ + data.size() - output_.writer().bytes_pushed() <= output_.writer().available_capacity()){
            cathe_[first_index_] = move(data);
        }else {
            cathe_[first_index_] = data.substr(0 , output_.writer().bytes_pushed()  + output_.writer().available_capacity() - first_index_);
        }

    }
}

uint64_t Reassembler::bytes_pending() const
{
    size_t ans{};
    size_t temp = output_.writer().bytes_pushed();
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
    for (const auto & it : cathe_){
        if (it.first == output_.writer().bytes_pushed()){
            output_.writer().push(move(it.second));
        }else if (it.first < output_.writer().bytes_pushed() && it.first + it.second.size() > output_.writer().bytes_pushed() ){
            output_.writer().push(it.second.substr(output_.writer().bytes_pushed() - it.first));
        }
    }
    if (output_.writer().bytes_pushed() == last_string_index_ ){
        cathe_.clear();
        output_.writer().close();
    }
}
