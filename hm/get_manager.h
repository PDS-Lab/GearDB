#ifndef LEVELDB_GET_MANAGER_H
#define LEVELDB_GET_MANAGER_H

//////
//Module function: get main module
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