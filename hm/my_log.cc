#include "../hm/my_log.h"

namespace leveldb{

    void init_log_file(){
#if GEARDB_DBBUG
        FILE* fp;
        fp = fopen(log_name.c_str(), "w");
        if(fp == nullptr) printf("log failed\n");
        fclose(fp);

#if 0
        fp = fopen(log_name2.c_str(), "w");
        if(fp == nullptr) printf("log failed\n");
        fclose(fp);
#endif
        fp = fopen(log_name3.c_str(), "w");
        if(fp == nullptr) printf("log failed\n");
        fclose(fp);

        fp = fopen(log_name4.c_str(), "w");
        if(fp == nullptr) printf("log failed\n");
        fclose(fp);
        MyLog4("compaction,read(MB),write(MB),time(s)\n");

        fp = fopen(log_name5.c_str(), "w");
        if(fp == nullptr) printf("log failed\n");
        fclose(fp);

        fp = fopen(log_name6.c_str(), "w");
        if(fp == nullptr) printf("log failed\n");
        fclose(fp);
#endif

    }
    
    void MyLog(const char* format, ...){
#if GEARDB_DBBUG
        va_list ap;
        va_start(ap, format);
        char buf[1024];
        vsprintf(buf, format, ap);
        va_end(ap);

        FILE* fp = fopen(log_name.c_str(), "a");
        if(fp == nullptr) printf("log failed\n");
        fprintf(fp, "%s", buf);
        fclose(fp);
#endif
    }

    void MyLog2(const char* format, ...){
#if GEARDB_DBBUG
        va_list ap;
        va_start(ap, format);
        char buf[1024];
        
        vsprintf(buf, format, ap);
            
        va_end(ap);

        FILE* fp = fopen(log_name2.c_str(), "a");
        if(fp == nullptr) printf("log failed\n");
        fprintf(fp, "%s", buf);
        fclose(fp);
#endif
    }
    
    void MyLog3(const char* format, ...){
#if GEARDB_DBBUG
        va_list ap;
        va_start(ap, format);
        char buf[1024];
        
        vsprintf(buf, format, ap);
            
        va_end(ap);

        FILE* fp = fopen(log_name3.c_str(), "a");
        if(fp == nullptr) printf("log failed\n");
        fprintf(fp, "%s", buf);
        fclose(fp);
#endif
    }

    void MyLog4(const char* format, ...){
#if GEARDB_DBBUG
        va_list ap;
        va_start(ap, format);
        char buf[1024];
        
        vsprintf(buf, format, ap);
            
        va_end(ap);

        FILE* fp = fopen(log_name4.c_str(), "a");
        if(fp == nullptr) printf("log failed\n");
        fprintf(fp, "%s", buf);
        fclose(fp);
#endif
    }

    void MyLog5(const char* format, ...){
#if GEARDB_DBBUG
        va_list ap;
        va_start(ap, format);
        char buf[1024];
        
        vsprintf(buf, format, ap);
            
        va_end(ap);

        FILE* fp = fopen(log_name5.c_str(), "a");
        if(fp == nullptr) printf("log failed\n");
        fprintf(fp, "%s", buf);
        fclose(fp);
#endif
    }

    void MyLog6(const char* format, ...){
#if GEARDB_DBBUG
        va_list ap;
        va_start(ap, format);
        char buf[1024];
        
        vsprintf(buf, format, ap);
            
        va_end(ap);

        FILE* fp = fopen(log_name6.c_str(), "a");
        if(fp == nullptr) printf("log failed\n");
        fprintf(fp, "%s", buf);
        fclose(fp);
#endif
    }

}