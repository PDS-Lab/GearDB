#include "../hm/container.h"
namespace leveldb{

    Container::Container(){
        Clear();
        estimate_size_ = 0;
    }

    Container::~Container(){

    }

    void Container::Add(const Slice& key, const Slice& value){
        char *key_buf = new char[key.size()];
        char *value_buf = new char[value.size()];

        memcpy(key_buf, key.data(), key.size());
        memcpy(value_buf, value.data(), value.size());
        kv_list_.push_back(std::make_pair(Slice(key_buf, key.size()),Slice(value_buf, value.size())));
        estimate_size_ += key.size();
        estimate_size_ += value.size();
    }

    void Container::Clear(){
        for(auto kv : kv_list_){
            delete kv.first.data();
            delete kv.second.data();
        }
        kv_list_.clear();
        estimate_size_ = 0;
    }

    Iterator* Container::NewIterator(const Comparator* icmp){
        return new ContainerIterator(&kv_list_, icmp);
    }

    const InternalKey* Container::Getsmallest(){
        if(estimate_size_==0){
            return NULL;
        }
        smallest_.DecodeFrom(kv_list_[0].first);
        return &smallest_;
    }

    const InternalKey* Container::Getlargest(){
        if(estimate_size_==0){
            return NULL;
        }
        largest_.DecodeFrom(kv_list_[kv_list_.size()-1].first);
        return &largest_;
    }



    ContainerIterator::ContainerIterator(std::vector<kv_pair> *kv_list, const Comparator* icmp) : icmp_(icmp){
        kv_list_=kv_list;
        list_size=kv_list_->size();
        pos_=0;

    }

    ContainerIterator::~ContainerIterator(){

    }

    bool ContainerIterator::Valid() const{
        return (pos_ < kv_list_->size()) && (pos_ >= 0);
    }

    void ContainerIterator::SeekToFirst(){
        pos_=0;
    }

    void ContainerIterator::Next(){
        assert(Valid());
        pos_++;
    }

    Slice ContainerIterator::key() const{
        assert(Valid());
        return (*kv_list_)[pos_].first;
    }

    Slice ContainerIterator::value() const{
        assert(Valid());
        return (*kv_list_)[pos_].second;
    }

    void ContainerIterator::Seek(const Slice &target) {
        uint32_t left = 0;
        uint32_t right = kv_list_->size();
        while (left < right) {
            uint32_t mid = (left + right) / 2;
            const Slice key = (*kv_list_)[mid].first;
            if (icmp_->Compare(key, target) < 0) {
                // Key at "mid.largest" is < "target".  Therefore all
                // files at or before "mid" are uninteresting.
                left = mid + 1;
            } else {
                // Key at "mid.largest" is >= "target".  Therefore all files
                // after "mid" are uninteresting.
                right = mid;
            }
        }
        pos_ = right;
    }

    void ContainerIterator::Prev() {
        assert(Valid());
        if(pos_ == 0){
            pos_ = kv_list_->size();
        }
        pos_--;
    }

    void ContainerIterator::SeekToLast() {
        pos_ = kv_list_->empty() ? 0 : kv_list_->size() - 1;
    }
}