#ifndef LEVELDB_GET_MANAGER_H
#define LEVELDB_GET_MANAGER_H

//////
//模块功能:获取自己管理的主模块
//////
namespace leveldb{
    class HMManager;

    class Singleton{
    public:
        ~Singleton();
        static HMManager* Gethmmanager();

    private:
        Singleton();

        static HMManager* hm_manager_;

        class Deletor{
        public:
            ~Deletor();
        };

        static Deletor deletor_;
    };
}

#endif //HM_LEVELDB_MANAGER_SINGLETON_H