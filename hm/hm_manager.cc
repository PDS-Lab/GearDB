#include <cstdint>
#include <fcntl.h>
#include <sys/time.h>

#include "../hm/hm_manager.h"


namespace leveldb{
    static uint64_t get_now_micros(){
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_sec) * 1000000 + tv.tv_usec;
    }

    static size_t random_number(size_t size) {
        return rand() % size;
    }

    HMManager::HMManager(const Comparator *icmp)
        :icmp_(icmp) {
        ssize_t ret;

        //ret = zbc_open(smr_filename, O_RDWR, &dev_);  //Open device without O_DIRECT
        ret = zbc_open(smr_filename, O_RDWR | O_DIRECT, &dev_);  //Open device with O_DIRECT; O_DIRECT means that Write directly to disk without cache
        if (ret != 0) {
            printf("error:%ld open failed!\n",ret);
            return ;
        }

        zbc_reset_zone(dev_,0,1);    //reset all zone

        ret = zbc_list_zones(dev_,0, ZBC_RO_ALL,&zone_, &zonenum_);  //get zone info
        if (ret != 0) {
            printf("error:%ld zbc_list_zones failed!\n",ret);
            return ;
        }

        bitmap_ = new BitMap(zonenum_);
        first_zonenum_ = set_first_zonenum();

        init_log_file();
        MyLog("\n  !!geardb!!  \n");
        MyLog("COM_WINDOW_SEQ:%d Verify_Table:%d Read_Whole_Table:%d Find_Table_Old:%d\n",COM_WINDOW_SEQ,Verify_Table,Read_Whole_Table,Find_Table_Old);
        MyLog("the first_zonenum_:%d zone_num:%ld\n",first_zonenum_,zonenum_);
        //////statistics
        delete_zone_num=0;
        all_table_size=0;
        kv_store_sector=0;
        kv_read_sector=0;
        max_zone_num=0;
        move_file_size=0;
        read_time=0;
        write_time=0;
        //////end
    }

    HMManager::~HMManager(){
        get_all_info();

        std::map<uint64_t, struct Ldbfile*>::iterator it=table_map_.begin();
        while(it!=table_map_.end()){
            delete it->second;
            it=table_map_.erase(it);
        }
        table_map_.clear();
        int i;
        for(i=0;i<config::kNumLevels;i++){
            std::vector<struct Zonefile*>::iterator iz=zone_info_[i].begin();
            while(iz!=zone_info_[i].end()){
                delete (*iz);
                iz=zone_info_[i].erase(iz);
            }
            zone_info_[i].clear();
        }
        if(zone_) {
            free(zone_);
        }
        if(dev_){
            zbc_close(dev_);
        }
        if(bitmap_){
            free(bitmap_);
        }

    }

    int HMManager::set_first_zonenum(){
        int i;
        for(i=0;i<zonenum_;i++){
            if(zone_[i].zbz_type==2 || zone_[i].zbz_type==3){
                return i;
            }
        }
        return 0;
    }

    ssize_t HMManager::hm_alloc_zone(){
        ssize_t i;
        for(i=first_zonenum_;i<zonenum_;i++){  //Traverse from the first sequential write zone
            if(bitmap_->get(i)==0){
                unsigned int num = 1;
                enum zbc_reporting_options ro = ZBC_RO_ALL;
                struct zbc_zone *zone=(struct zbc_zone *)malloc(sizeof(struct zbc_zone));
                zbc_report_zones(dev_,zone_[i].zbz_start,ro,zone,&num);
                if(zone->zbz_write_pointer != zone_[i].zbz_start){
                    MyLog("alloc error: zone:%ld wp:%ld\n",i,zone->zbz_write_pointer);
                    zbc_reset_zone(dev_,zone_[i].zbz_start,0);
                    if(zone) free(zone);
                    continue;
                }
                if(zone) free(zone);
                bitmap_->set(i);
                zone_[i].zbz_write_pointer=zone_[i].zbz_start;
                return i;
            }
        }
        printf("hm_alloc_zone failed!\n");
        return -1;
    }
    
    ssize_t HMManager::hm_free_zone(uint64_t zone){
        ssize_t ret;
        ret =zbc_reset_zone(dev_,zone_[zone].zbz_start,0);
        if(ret!=0){
            MyLog("reset zone:%ld faild! error:%ld\n",zone,ret);
        }
        bitmap_->clr(zone);
        zone_[zone].zbz_write_pointer=zone_[zone].zbz_start;
    }

    ssize_t HMManager::hm_alloc(int level,uint64_t size){
        uint64_t need_size=(size%PHY_SECTOR_SIZE)? (size/PHY_SECTOR_SIZE+1)*(PHY_SECTOR_SIZE/512) :size/512;
        uint64_t write_zone=0;
        if(zone_info_[level].empty()){
            write_zone=hm_alloc_zone();
            struct Zonefile* zf=new Zonefile(write_zone);
            zone_info_[level].push_back(zf);

            if(get_zone_num()>max_zone_num){
                max_zone_num=get_zone_num();
            }
        
            return 1;
        }
        write_zone=zone_info_[level][zone_info_[level].size()-1]->zone;
        if((zone_[write_zone].zbz_length-(zone_[write_zone].zbz_write_pointer-zone_[write_zone].zbz_start))<need_size) {//The current written zone can't write
            write_zone=hm_alloc_zone();
            struct Zonefile* zf=new Zonefile(write_zone);
            zone_info_[level].push_back(zf);
            if(get_zone_num()>max_zone_num){
                max_zone_num=get_zone_num();
            }
        }
        return 1;
    }

    ssize_t HMManager::hm_write(int level,uint64_t filenum,const void *buf,uint64_t count){
        
        hm_alloc(level,count);
        void *w_buf=NULL;
        uint64_t write_zone=zone_info_[level][zone_info_[level].size()-1]->zone;
        uint64_t sector_count;
        uint64_t sector_ofst=zone_[write_zone].zbz_write_pointer;
        ssize_t ret;

        uint64_t write_time_begin=get_now_micros();
        if(count%PHY_SECTOR_SIZE==0){
            sector_count=count/512;
            ret=zbc_pwrite(dev_, buf, sector_count, sector_ofst);
        }
        else{
            sector_count=(count/PHY_SECTOR_SIZE+1)*(PHY_SECTOR_SIZE/512);  //Align with physical block
            //w_buf=(void *)malloc(sector_count*512);
            ret=posix_memalign(&w_buf,MEMALIGN_SIZE,sector_count*512);
            if(ret!=0){
                printf("error:%ld posix_memalign falid!\n",ret);
                return -1;
            }
            memset(w_buf,0,sector_count*512);
            memcpy(w_buf,buf,count);
            ret=zbc_pwrite(dev_, w_buf, sector_count, sector_ofst);
            free(w_buf);
        }
        if(ret<=0){
            printf("error:%ld hm_write falid! table:%ld\n",ret,filenum);
            return -1;
        }
        uint64_t write_time_end=get_now_micros();
        write_time += (write_time_end-write_time_begin);

        zone_[write_zone].zbz_write_pointer +=sector_count;
        struct Ldbfile *ldb= new Ldbfile(filenum,write_zone,sector_ofst,count,level);
        table_map_.insert(std::pair<uint64_t, struct Ldbfile*>(filenum,ldb));
        zone_info_[level][zone_info_[level].size()-1]->add_table(ldb);
        all_table_size += ldb->size;
        kv_store_sector += sector_count;

        MyLog("write table:%ld to level-%d zone:%ld of size:%ld bytes ofst:%ld sect:%ld next:%ld\n",filenum,level,write_zone,count,sector_ofst,sector_count,sector_ofst+sector_count);
        
        return ret*512;
    }

    ssize_t HMManager::hm_read(uint64_t filenum,void *buf,uint64_t count, uint64_t offset){
        void *r_buf=NULL;
        uint64_t sector_count;
        uint64_t sector_ofst;
        uint64_t de_ofst;
        ssize_t ret;
        uint64_t read_time_begin=get_now_micros();

        std::map<uint64_t, struct Ldbfile*>::iterator it;
        it=table_map_.find(filenum);
        if(it==table_map_.end()){
            printf(" table_map_ can't find table:%ld!\n",filenum);
            return -1;
        }
        
        sector_ofst=it->second->offset+offset/512;
        de_ofst=offset - (offset/512)*512;

        sector_count=((count+de_ofst)%512) ? (count+de_ofst)/512+1 : (count+de_ofst)/512;   //Align with logical block

        //r_buf=(void *)malloc(sector_count*512);
        ret=posix_memalign(&r_buf,MEMALIGN_SIZE,sector_count*512);
        if(ret!=0){
            printf("error:%ld posix_memalign falid!\n",ret);
            return -1;
        }
        memset(r_buf,0,sector_count*512);
        ret=zbc_pread(dev_, r_buf, sector_count,sector_ofst);
        memcpy(buf,((char *)r_buf)+de_ofst,count);
        free(r_buf);
        if(ret<=0){
            printf("error:%ld hm_read falid!\n",ret);
            return -1;
        }
        
        uint64_t read_time_end=get_now_micros();
        read_time +=(read_time_end-read_time_begin);
        kv_read_sector += sector_count;
        //MyLog("read table:%ld of size:%ld bytes file:%ld bytes\n",filenum,count,it->second->size);
        return count;
    }

    ssize_t HMManager::hm_delete(uint64_t filenum){
        std::map<uint64_t, struct Ldbfile*>::iterator it;
        it=table_map_.find(filenum);
        if(it!=table_map_.end()){
            struct Ldbfile *ldb=it->second;
            table_map_.erase(it);
            int level=ldb->level;
            uint64_t zone_id=ldb->zone;
            std::vector<struct Zonefile*>::iterator iz=zone_info_[level].begin();
            for(;iz!=zone_info_[level].end();iz++){
                if(zone_id==(*iz)->zone){
                    (*iz)->delete_table(ldb);
                    if((*iz)->ldb.empty() && (zone_[zone_id].zbz_write_pointer-zone_[zone_id].zbz_start) > 128*2048){
                        struct Zonefile* zf=(*iz);
                        zone_info_[level].erase(iz);
                        if(is_com_window(level,zone_id)){
                            std::vector<struct Zonefile*>::iterator ic=com_window_[level].begin();
                            for(;ic!=com_window_[level].end();ic++){
                                if((*ic)->zone==zone_id){
                                    com_window_[level].erase(ic);
                                    break;
                                }
                            }
                        }
                        delete zf;
                        hm_free_zone(zone_id);
                        MyLog("delete zone:%ld from level-%d\n",zone_id,level);
                        delete_zone_num++;
                        
                    }
                    break;
                }
            }
            MyLog("delete table:%ld from level-%d zone:%ld of size:%ld MB\n",filenum,level,zone_id,ldb->size/1048576);
            all_table_size -= ldb->size;
            delete ldb;
        }
        return 1;
    }

    ssize_t HMManager::move_file(uint64_t filenum,int to_level){
        void *r_buf=NULL;
        ssize_t ret;
        std::map<uint64_t, struct Ldbfile*>::iterator it;
        it=table_map_.find(filenum);
        if(it==table_map_.end()){
            printf("error:move file failed! no find file:%ld\n",filenum);
            return -1;
        }
        struct Ldbfile *ldb=it->second;
        uint64_t sector_count=((ldb->size+PHY_SECTOR_SIZE-1)/PHY_SECTOR_SIZE)*(PHY_SECTOR_SIZE/512);
        uint64_t sector_ofst=ldb->offset;
        uint64_t file_size=ldb->size;
        int old_level=ldb->level;
        
        uint64_t read_time_begin=get_now_micros();
        //r_buf=(void *)malloc(sector_count*512);
        ret=posix_memalign(&r_buf,MEMALIGN_SIZE,sector_count*512);
        if(ret!=0){
            printf("error:%ld posix_memalign falid!\n",ret);
            return -1;
        }
        memset(r_buf,0,sector_count*512);
        ret=zbc_pread(dev_, r_buf, sector_count,sector_ofst);
        if(ret<=0){
            printf("error:%ld z_read falid!\n",ret);
            return -1;
        }
        uint64_t read_time_end=get_now_micros();
        read_time +=(read_time_end-read_time_begin);

        hm_delete(filenum);
        kv_read_sector += sector_count;

        uint64_t write_time_begin=get_now_micros();

        hm_alloc(to_level,file_size);
        uint64_t write_zone=zone_info_[to_level][zone_info_[to_level].size()-1]->zone;
        sector_ofst=zone_[write_zone].zbz_write_pointer;
        ret=zbc_pwrite(dev_, r_buf, sector_count, sector_ofst);
        if(ret<=0){
            printf("error:%ld zbc_pwrite falid!\n",ret);
            return -1;
        }
        uint64_t write_time_end=get_now_micros();
        write_time += (write_time_end-write_time_begin);

        zone_[write_zone].zbz_write_pointer +=sector_count;
        ldb= new Ldbfile(filenum,write_zone,sector_ofst,file_size,to_level);
        table_map_.insert(std::pair<uint64_t, struct Ldbfile*>(filenum,ldb));
        zone_info_[to_level][zone_info_[to_level].size()-1]->add_table(ldb);
        
        free(r_buf);
        kv_store_sector += sector_count;
        move_file_size += file_size;
        all_table_size += file_size;

        MyLog("move table:%ld from level-%d to level-%d zone:%ld of size:%ld MB\n",filenum,old_level,to_level,write_zone,file_size/1048576);
        return 1;
    }

    struct Ldbfile* HMManager::get_one_table(uint64_t filenum){
        std::map<uint64_t, struct Ldbfile*>::iterator it;
        it=table_map_.find(filenum);
        if(it==table_map_.end()){
            printf("error:no find file:%ld\n",filenum);
            return NULL;
        }
        return it->second;
    }

    void HMManager::get_zone_table(uint64_t filenum,std::vector<struct Ldbfile*> **zone_table){
        std::map<uint64_t, struct Ldbfile*>::iterator it;
        it=table_map_.find(filenum);
        if(it==table_map_.end()){
            printf("error:no find file:%ld\n",filenum);
            return ;
        }

        int level=it->second->level;
        uint64_t zone_id=it->second->zone;
        std::vector<struct Zonefile*>::iterator iz;
        for(iz=zone_info_[level].begin();iz!=zone_info_[level].end();iz++){
            if((*iz)->zone==zone_id){
                *zone_table=&((*iz)->ldb);
                return;
            }
        }

    }

    bool HMManager::trivial_zone_size_move(uint64_t filenum){
        std::map<uint64_t, struct Ldbfile*>::iterator it;
        it=table_map_.find(filenum);
        if(it==table_map_.end()){
            printf("error:no find file:%ld\n",filenum);
            return false;
        }

        uint64_t zone_id=it->second->zone;
        if((zone_[zone_id].zbz_length-(zone_[zone_id].zbz_write_pointer-zone_[zone_id].zbz_start)) < 64*2048){ //The remaining free space is less than 64MB, triggering
            return true;
        }
        else return false;
    }

    void HMManager::move_zone(uint64_t filenum){
        std::map<uint64_t, struct Ldbfile*>::iterator it;
        it=table_map_.find(filenum);
        if(it==table_map_.end()){
            printf("error:no find file:%ld\n",filenum);
            return ;
        }

        int level=it->second->level;
        uint64_t zone_id=it->second->zone;
        std::vector<struct Zonefile*>::iterator iz;
        struct Zonefile* zf;

        if(is_com_window(level,zone_id)){
            std::vector<struct Zonefile*>::iterator ic=com_window_[level].begin();
            for(;ic!=com_window_[level].end();ic++){
                if((*ic)->zone==zone_id){
                    com_window_[level].erase(ic);
                    break;
                }
            }
        }
        for(iz=zone_info_[level].begin();iz!=zone_info_[level].end();iz++){
            if((*iz)->zone==zone_id){
                zf=(*iz);
                zone_info_[level].erase(iz);
                break;
            }
        }
        MyLog("before move zone:[");
        for(int i=0;i<zone_info_[level+1].size();i++){
            MyLog("%ld ",zone_info_[level+1][i]->zone);
        }
        MyLog("]\n");

        int size=zone_info_[level+1].size();
        if(size==0) size=1;
        zone_info_[level+1].insert(zone_info_[level+1].begin()+(size-1),zf);

        for(int i=0;i<zf->ldb.size();i++){
            zf->ldb[i]->level=level+1;
        }

        MyLog("move zone:%d table:[",zone_id);
        for(int i=0;i<zf->ldb.size();i++){
            MyLog("%ld ",zf->ldb[i]->table);
        }
        MyLog("] to level:%d\n",level+1);

        MyLog("end move zone:[");
        for(int i=0;i<zone_info_[level+1].size();i++){
            MyLog("%ld ",zone_info_[level+1][i]->zone);
        }
        MyLog("]\n");
    }


    void HMManager::update_com_window(int level){
        ssize_t window_num=adjust_com_window_num(level);
        if(COM_WINDOW_SEQ) {
            set_com_window_seq(level,window_num);
        }
        else{
            set_com_window(level,window_num);
        }
        
    }

    ssize_t HMManager::adjust_com_window_num(int level){
        ssize_t window_num=0;
        switch (level){
            case 0:
            case 1:
            case 2:
                window_num = zone_info_[level].size();   //1,2 level's compaction window number is all the level
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                window_num = zone_info_[level].size()/COM_WINDOW_SCALE; //other level compaction window number is 1/COM_WINDOW_SCALE
                break;
            default:
                break;
        }
        return window_num;
    }

    void HMManager::set_com_window(int level,int num){
        int i;
        if(level==1||level==2){
            com_window_[level].clear();
            for(i=0;i<zone_info_[level].size();i++){
                com_window_[level].push_back(zone_info_[level][i]);
            }
            return;
        }
        if(com_window_[level].size() >= num){
            return;
        }
        size_t ran_num;
        for(i=com_window_[level].size();i<num;i++){
            while(1){
                ran_num=random_number(zone_info_[level].size()-1);
                if(!is_com_window(level,zone_info_[level][ran_num]->zone)){
                    break;
                }
            }
            com_window_[level].push_back(zone_info_[level][ran_num]);
        }
    }

    void HMManager::set_com_window_seq(int level,int num){
        int i;
        if(level==1||level==2){
            com_window_[level].clear();
            for(i=0;i<zone_info_[level].size();i++){
                com_window_[level].push_back(zone_info_[level][i]);
            }
            return;
        }
        if(com_window_[level].size() >= num){
            return;
        }
        com_window_[level].clear();
        for(i=0;i<num;i++){
            com_window_[level].push_back(zone_info_[level][i]);
        }

    }

    bool HMManager::is_com_window(int level,uint64_t zone){
        std::vector<struct Zonefile*>::iterator it;
        for(it=com_window_[level].begin();it!=com_window_[level].end();it++){
            if((*it)->zone==zone){
                return true;
            }
        }
        return false;
    }

    void HMManager::get_com_window_table(int level,std::vector<struct Ldbfile*> *window_table){
        std::vector<struct Zonefile*>::iterator iz;
        std::vector<struct Ldbfile*>::iterator it;
        for(iz=com_window_[level].begin();iz!=com_window_[level].end();iz++){
            for(it=(*iz)->ldb.begin();it!=(*iz)->ldb.end();it++){
                window_table->push_back((*it));
            }
        }

    }







    //////statistics
    uint64_t HMManager::get_zone_num(){
        uint64_t num=0;
        int i;
        for(i=0;i<config::kNumLevels;i++){
            num += zone_info_[i].size();
        }
        return num;
    }

    void HMManager::get_one_level(int level,uint64_t *table_num,uint64_t *table_size){
        std::vector<struct Zonefile*>::iterator it;
        uint64_t num=0;
        uint64_t size=0;
        for(it=zone_info_[level].begin();it!=zone_info_[level].end();it++){
            num += (*it)->ldb.size();
            size += (*it)->get_all_file_size();
        }
        *table_num = num;
        *table_size = size;
    }

    void HMManager::get_per_level_info(){
        int i;
        uint64_t table_num=0;
        uint64_t table_size=0;
        float percent=0;
        int zone_num=0;
        uint64_t zone_id;

        for(i=0;i<config::kNumLevels;i++){
            get_one_level(i,&table_num,&table_size);
            if(table_size == 0){
                percent = 0;
            }
            else {
                zone_num=zone_info_[i].size();
                zone_id=zone_info_[i][zone_num-1]->zone;
                percent=100.0*table_size/((zone_num-1)*256.0*1024*1024+(zone_[zone_id].zbz_write_pointer - zone_[zone_id].zbz_start)*512.0);
            }
            MyLog("Level-%d zone_num:%d table_num:%ld table_size:%ld MB percent:%.2f %%\n",i,zone_info_[i].size(),table_num,table_size/1048576,percent);
        }
    }

    void HMManager::get_valid_info(){
        
        MyLog("write_zone:%ld delete_zone_num:%ld max_zone_num:%ld table_num:%ld table_size:%ld MB\n",get_zone_num(),delete_zone_num,max_zone_num,table_map_.size(),all_table_size/1048576);
        get_per_level_info();
        uint64_t table_num;
        uint64_t table_size;
        uint64_t zone_id;
        float percent;
        std::vector<struct Zonefile*>::iterator it;
        int i;
        for(i=0;i<config::kNumLevels;i++){
            if(zone_info_[i].size() != 0){
                for(it=zone_info_[i].begin();it!=zone_info_[i].end();it++){
                    zone_id=(*it)->zone;
                    table_num=(*it)->ldb.size();
                    table_size=(*it)->get_all_file_size();
                    percent=100.0*table_size/(256.0*1024*1024);
                    MyLog("Level-%d zone_id:%ld table_num:%ld valid_size:%ld MB percent:%.2f %% \n",i,zone_id,table_num,table_size/1048576,percent);
                }
                
            }
        }

    }

    void HMManager::get_all_info(){
        uint64_t disk_size=(get_zone_num())*zone_[first_zonenum_].zbz_length;

        MyLog("\nget all data!\n");
        MyLog("table_all_size:%ld MB kv_read_sector:%ld MB kv_store_sector:%ld MB disk_size:%ld MB \n",all_table_size/(1024*1024),\
            kv_read_sector/2048,kv_store_sector/2048,disk_size/2048);
        MyLog("read_time:%.1f s write_time:%.1f s read:%.1f MB/s write:%.1f MB/s\n",1.0*read_time*1e-6,1.0*write_time*1e-6,\
            (kv_read_sector/2048.0)/(read_time*1e-6),(kv_store_sector/2048.0)/(write_time*1e-6));
        get_valid_info();
        MyLog("\n");
        
    }

    void HMManager::get_valid_data(){
        
        MyLog2("level,zone_id,table_num,valid_size(MB),percent(%%)\n");
        uint64_t table_num;
        uint64_t table_size;
        uint64_t zone_id;
        float percent;
        std::vector<struct Zonefile*>::iterator it;
        int i;
        for(i=0;i<config::kNumLevels;i++){
            if(zone_info_[i].size() != 0){
                for(it=zone_info_[i].begin();it!=zone_info_[i].end();it++){
                    zone_id=(*it)->zone;
                    table_num=(*it)->ldb.size();
                    table_size=(*it)->get_all_file_size();
                    percent=100.0*table_size/(256.0*1024*1024);
                    MyLog2("%d,%ld,%ld,%ld,%.2f\n",i,zone_id,table_num,table_size/1048576,percent);
                }
                
            }
        }
    }

    void HMManager::get_my_info(int num){
        MyLog6("\nnum:%d table_size:%ld MB kv_read_sector:%ld MB kv_store_sector:%ld MB zone_num:%ld max_zone_num:%ld move_size:%ld MB\n",num,all_table_size/(1024*1024),\
            kv_read_sector/2048,kv_store_sector/2048,get_zone_num(),max_zone_num,move_file_size/(1024*1024));
        MyLog6("read_time:%.1f s write_time:%.1f s read:%.1f MB/s write:%.1f MB/s\n",1.0*read_time*1e-6,1.0*write_time*1e-6,\
            (kv_read_sector/2048.0)/(read_time*1e-6),(kv_store_sector/2048.0)/(write_time*1e-6));
        get_valid_all_data(num);
    }

    void HMManager::get_valid_all_data(int num){
        uint64_t disk_size=(get_zone_num())*zone_[first_zonenum_].zbz_length;

        MyLog3("\nnum:%d\n",num);
        MyLog3("table_all_size:%ld MB kv_read_sector:%ld MB kv_store_sector:%ld MB disk_size:%ld MB \n",all_table_size/(1024*1024),\
            kv_read_sector/2048,kv_store_sector/2048,disk_size/2048);
        MyLog3("read_time:%.1f s write_time:%.1f s read:%.1f MB/s write:%.1f MB/s\n",1.0*read_time*1e-6,1.0*write_time*1e-6,\
            (kv_read_sector/2048.0)/(read_time*1e-6),(kv_store_sector/2048.0)/(write_time*1e-6));
        MyLog3("write_zone:%ld delete_zone_num:%ld max_zone_num:%ld table_num:%ld table_size:%ld MB\n",get_zone_num(),delete_zone_num,max_zone_num,table_map_.size(),all_table_size/1048576);
        uint64_t table_num;
        uint64_t table_size;
        int zone_num=0;
        uint64_t zone_id;
        float percent;
        std::vector<struct Zonefile*>::iterator it;
        int i;
        for(i=0;i<config::kNumLevels;i++){
            get_one_level(i,&table_num,&table_size);
            if(table_size == 0){
                percent = 0;
            }
            else {
                zone_num=zone_info_[i].size();
                zone_id=zone_info_[i][zone_num-1]->zone;
                percent=100.0*table_size/((zone_num-1)*256.0*1024*1024+(zone_[zone_id].zbz_write_pointer - zone_[zone_id].zbz_start)*512.0);
            }
            MyLog3("Level-%d zone_num:%d table_num:%ld table_size:%ld MB percent:%.2f %% \n",i,zone_info_[i].size(),table_num,table_size/1048576,percent);
        }
        MyLog3("level,zone_id,table_num,valid_size(MB),percent(%%)\n");
        for(i=0;i<config::kNumLevels;i++){
            if(zone_info_[i].size() != 0){
                for(it=zone_info_[i].begin();it!=zone_info_[i].end();it++){
                    zone_id=(*it)->zone;
                    table_num=(*it)->ldb.size();
                    table_size=(*it)->get_all_file_size();
                    percent=100.0*table_size/(256.0*1024*1024);
                    MyLog3("%d,%ld,%ld,%ld,%.2f\n",i,zone_id,table_num,table_size/1048576,percent);
                }
                
            }
        }
    }




    //////end

    

    
    




}