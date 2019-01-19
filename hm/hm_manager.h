#ifndef LEVELDB_HM_MANAGER_H
#define LEVELDB_HM_MANAGER_H

//////
//模块功能:自己管理的主模块
//////

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <map>
#include <vector>  

#include "../db/dbformat.h"
#include "../hm/my_log.h"
#include "../hm/BitMap.h"
#include "../hm/hm_status.h"


extern "C" {
#include <libzbc/zbc.h>
}

namespace leveldb{

    class HMManager {
    public:
        HMManager(const Comparator *icmp);
        ~HMManager();
        
        ssize_t hm_write(int level,uint64_t filenum,const void *buf,uint64_t count);   //向某层写入一个ldb文件
        ssize_t hm_read(uint64_t filenum,void *buf,uint64_t count, uint64_t offset);   //对某个ldb文件进行读取
        ssize_t hm_delete(uint64_t filenum);                                           //删除某个ldb文件
        ssize_t move_file(uint64_t filenum,int to_level);                              //迁移某个ldb文件到某层
        struct Ldbfile* get_one_table(uint64_t filenum);


        void get_table(std::map<uint64_t, struct Ldbfile*> **table_map){ *table_map=&table_map_; };  //获取全部table
        
        //////dump机制相关
        void get_zone_table(uint64_t filenum,std::vector<struct Ldbfile*> **zone_table);
        bool trivial_zone_size_move(uint64_t filenum);
        void move_zone(uint64_t filenum);
        //////

        //////窗口相关
        void update_com_window(int level);
        void get_com_window_table(int level,std::vector<struct Ldbfile*> *window_table);
        ssize_t adjust_com_window_num(int level);
        void set_com_window(int level,int num);
        void set_com_window_seq(int level,int num);
        //////

        //////统计
        uint64_t get_zone_num();
        void get_one_level(int level,uint64_t *table_num,uint64_t *table_size);
        void get_per_level_info();
        void get_valid_info();
        void get_all_info();
        void get_valid_data();
        void get_my_info(int num);
        void get_valid_all_data(int num);

        //////end

    private:
        BitMap *bitmap_;

        struct zbc_device *dev_;
        struct zbc_zone  *zone_;
        unsigned int zonenum_;
        int first_zonenum_;

        const InternalKeyComparator icmp_;

        std::map<uint64_t, struct Ldbfile*> table_map_;  //<文件number，ldb信息>
        std::vector<struct Zonefile*> zone_info_[config::kNumLevels];  //每层zone的集合
        std::vector<struct Zonefile*> com_window_[config::kNumLevels];

        //////统计
        uint64_t delete_zone_num;
        uint64_t all_table_size;
        uint64_t kv_store_sector;
        uint64_t kv_read_sector;
        uint64_t max_zone_num;
        uint64_t move_file_size;
        uint64_t read_time;
        uint64_t write_time;
        //////end

        int set_first_zonenum();
        ssize_t hm_alloc(int level,uint64_t size);
        ssize_t hm_alloc_zone();
        ssize_t hm_free_zone(uint64_t zone);

        //////
        bool is_com_window(int level,uint64_t zone);
        //////

    };




}

#endif 
