#ifndef HM_LEVELDB_CONTAINER_H
#define HM_LEVELDB_CONTAINER_H

#include <vector>
#include "../include/leveldb/slice.h"
#include "../include/leveldb/iterator.h"
#include "../include/leveldb/status.h"
#include "../db/dbformat.h"

namespace leveldb{

    typedef std::pair<Slice , Slice > kv_pair;
    class Container{
    public:
        Container();

        ~Container();

        void Add(const Slice& key, const Slice& value);

        void Clear();

        bool empty(){return kv_list_.empty();}

        uint64_t EstimateSize(){return estimate_size_;}
        uint64_t Size(){return kv_list_.size();}

        const InternalKey* Getsmallest();
        const InternalKey* Getlargest();

        Iterator* NewIterator(const Comparator* icmp);

    private:
        std::vector<kv_pair> kv_list_;
        uint64_t estimate_size_;
        InternalKey smallest_;
        InternalKey largest_;
        

    };

    class ContainerIterator : public Iterator{
    public:
        ContainerIterator(std::vector<kv_pair> *kv_list, const Comparator*icmp);
        ~ContainerIterator();
        virtual bool Valid() const;
        virtual void SeekToFirst();
        virtual void Next();
        virtual Slice key() const;
        virtual Slice value() const;

        virtual void SeekToLast();
        virtual void Seek(const Slice& target);
        virtual void Prev();
        virtual Status status() const{return Status::OK();}


    private:
        const Comparator* icmp_;
        std::vector<kv_pair> *kv_list_;
        size_t list_size;
        size_t pos_;
    };
}

#endif //HM_LEVELDB_CONTAINER_H