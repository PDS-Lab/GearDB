#ifndef LEVELDB_HM_STATUS_H
#define LEVELDB_HM_STATUS_H

//////
//模块功能:一些变量和结构
//////

#include <vector>

#define PHY_SECTOR_SIZE 4096   //物理块大小
#define COM_WINDOW_SCALE 4    //窗口占该层zone的总个数的比例
#define HAVE_WINDOW_SCALE 4   //该层的数据达到阈值的1/HAVE_WINDOW_SCALE才有窗口

#define COM_WINDOW_SEQ 1      //0表示窗口选择随机，1表示窗口选择顺序

#define MEMALIGN_SIZE (sysconf(_SC_PAGESIZE))     //使用posix_memalign申请内存时对齐的大小

#define Verify_Table 1        //确认SSTable是否有用机制，每写入磁盘一个SSTable，它都会读该文件的句柄加入leveldb自己的cache，这是leveldb自己的机制；1表示有该机制，0表示无该机制。

#define Read_Whole_Table 1    //1表示读SSTable时整个文件一起读出来，0表示读操作时一个key_value读

#define Find_Table_Old  1     //1表示查找key-value时，跟原来一样，一个key-value读；0表示查找key-value和Read_Whole_Table参数一样

namespace leveldb {
    
    static const char smr_filename[]="";   //e.g. smr_filename[]="/dev/sdb1";

    struct Ldbfile {     //文件和在zone的位置关联结构
        uint64_t table;  //文件编号
        uint64_t zone;   //文件所在zone编号
        uint64_t offset; //文件在zone的地址
        uint64_t size;   //文件大小
        int      level;  //文件所在层数

        Ldbfile(uint64_t a,uint64_t b,uint64_t c,uint64_t d,int e):table(a),zone(b),offset(c),size(d),level(e){};
        ~Ldbfile(){};
    };

    struct Zonefile {    //zone内的ldb文件管理
        uint64_t zone;
        
        std::vector<struct Ldbfile*> ldb;
        Zonefile(uint64_t a):zone(a){};
        ~Zonefile(){};

        void add_table(struct Ldbfile* file){
            ldb.push_back(file);
        }
        void delete_table(struct Ldbfile* file){
            std::vector<struct Ldbfile*>::iterator it;
            for(it=ldb.begin();it!=ldb.end();){
                if((*it)==file){
                    ldb.erase(it);
                    return;
                }
                else it++;
            }
        }
        uint64_t get_all_file_size(){
            uint64_t size=0;
            std::vector<struct Ldbfile*>::iterator it;
            for(it=ldb.begin();it!=ldb.end();it++){
                size += (*it)->size;
            }
            return size;
        }
    };




}






#endif