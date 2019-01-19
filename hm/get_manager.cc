#include "../hm/get_manager.h"
#include "../hm/hm_manager.h"
#include "../include/leveldb/options.h"
#include "../hm/my_log.h"

namespace leveldb{
    Singleton::Singleton() {}

    Singleton::~Singleton() {}

    HMManager* Singleton::Gethmmanager() {
        if(hm_manager_ == NULL){
            hm_manager_ = new HMManager(Options().comparator);
        }
        return hm_manager_;
    }

    HMManager* Singleton::hm_manager_ = NULL;

    Singleton::Deletor::~Deletor() {
        if(hm_manager_ != NULL) delete hm_manager_;
    }

    Singleton::Deletor Singleton::deletor_;
}
