#ifndef LEVELDB_HM_LOG_H
#define LEVELDB_HM_LOG_H

//////
//Module function: output debug and statistics
//////

#include <string>
#include <cstdio>
#include <cstdarg>

#define GEARDB_DBBUG 0   //1 output some information to the file; 0 cancel

namespace leveldb{
    

    const std::string log_name("MyLOG");
    const std::string log_name2("Valid_DATA.csv");
    const std::string log_name3("ALL_Valid_Data");
    const std::string log_name4("Compaction_time.csv");
    const std::string log_name5("OP_LOG");
    const std::string log_name6("OP_DATA");
    
    
    void init_log_file();
    void MyLog(const char* format, ...);
    void MyLog2(const char* format, ...);
    void MyLog3(const char* format, ...);
    void MyLog4(const char* format, ...);
    void MyLog5(const char* format, ...);
    void MyLog6(const char* format, ...);
        

}

#endif //HM_LEVELDB_HM_DEBUG_H