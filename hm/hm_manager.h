#ifndef LEVELDB_HM_MANAGER_H
#define LEVELDB_HM_MANAGER_H

//////
//Module function: Main module
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
        
        ssize_t hm_write(int level,uint64_t filenum,const void *buf,uint64_t count);   //write a SSTable file to a level
        ssize_t hm_read(uint64_t filenum,void *buf,uint64_t count, uint64_t offset);   //read a SSTable file
        ssize_t hm_delete(uint64_t filenum);                                           //delete a SSTable file
        ssize_t move_file(uint64_t filenum,int to_level);                              //move a SSTable file
        struct Ldbfile* get_one_table(uint64_t filenum);                               //get a SSTable file pointer


        void get_table(std::map<uint64_t, struct Ldbfile*> **table_map){ *table_map=&table_map_; };  //get table_map
        
        //////dump relation
        void get_zone_table(uint64_t filenum,std::vector<struct Ldbfile*> **zone_table);
        bool trivial_zone_size_move(uint64_t filenum);
        void move_zone(uint64_t filenum);
        //////

        //////compaction relation
        void update_com_window(int level);
        void get_com_window_table(int level,std::vector<struct Ldbfile*> *window_table);
        ssize_t adjust_com_window_num(int level);
        void set_com_window(int level,int num);
        void set_com_window_seq(int level,int num);
        //////

        //////statistics
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

        std::map<uint64_t, struct Ldbfile*> table_map_;  //<file number, metadate pointer>
        std::vector<struct Zonefile*> zone_info_[config::kNumLevels];  //each level of zone
        std::vector<struct Zonefile*> com_window_[config::kNumLevels]; //each level of compaction window

        //////statistics
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
