# GearDB
## 1 Introduction
GearDB is the implementation code of the paper "GearDB: A GC-free Key-Value Store on HM-SMR Drives with Gear Compaction", which is modified based on [LeveldDB](https://github.com/google/leveldb) to finally realize its own functions.


## 2 Compilation and Run
### 2.1 Requiremensts
GearDB requires the [**Libzbc**](https://github.com/hgst/libzbc) tool to replace the disk's read and write interfaces.So you must first download and install [**Libzbc**](https://github.com/hgst/libzbc).
[**Libzbc**](https://github.com/hgst/libzbc)'s installation steps：
```
> git clone https://github.com/hgst/libzbc  
> sh ./autogen.sh
> ./configure
> make
> make install    //necessary
```

### 2.2 Compilation and Run 
1. If you use shingled magnetic recording drives as the storage medium, you don't need to take this step; If you are using HDD to simulate Host-managed shingled magnetic recording drives (HM-SMR), you need to use libzbc for simulation operations：  
```
> cd libzbc
> zbc_set_zones /dev/sdb1 set_sz 256 256  //Block device /dev/sdb1 is an example
```
2. Select the path of HM-SMR，e.g. /dev/sdb1. You need to add in the /GearDB/hm/hm_status.h file:
```
static const char smr_filename[]="/dev/sdb1";
```
1. GearDB's Compilation likes leveldb, we modified a little bit of the Makefile:
```
> make -j4
```
1. Then you can use out-static/db_bench for testing, or do more with  out-static/libleveldb.a.

## 3 Different from [LeveldDB](https://github.com/google/leveldb)
1. We modify it based on LevelDB. If you want to get the source version, you can do the following:
```
> git clone https://github.com/google/leveldb
> cd leveldb
> git checkout  23162ca1c6d891a9c5fe0e0fab1193cd54ed1b4f
```
Another way, you can open the link: <https://github.com/google/leveldb/tree/23162ca1c6d891a9c5fe0e0fab1193cd54ed1b4f>  
Then download. 

2. Added file: hm/*。
3. Modified file:  
```
db/builder.cc
db/builder.h
db/db_bench.cc
db/db_impl.cc
db/db_impl.h
db/leveldbutil.cc
db/table_cache.cc
db/table_cache.h
db/version_set.cc
db/version_set.h
include/leveldb/db.h
include/leveldb/env.h
util/env_posix.cc
util/options.cc
build_detect_platform
Makefile
```
## 4 Acknowledgement
Thanks to [LeveldDB](https://github.com/google/leveldb) and [Libzbc](https://github.com/hgst/libzbc)! We used them to complete our experiments.
## 5 Attentions
1. This version does not support database persistence. This means that if the existing database is opened after the program finishes running, it will get an error. We implemented persistence, but it made the operation a bit more cumbersome, so it was not given. For example, when testing in [YCSB](https://github.com/brianfrankcooper/YCSB.git), it needs to support the persistence of the database. More importantly, its operation process is cumbersome. If you need to support the persistent version, you can contact *993096281@qq.com*.
2. Our focus is on academic research. In the case described in the paper, there is no problem with running, but there is no guarantee that there will be no bugs under any circumstances. It is for reference, learning and communication only.
## 6 Contributors
- Ting Yao (tingyao@hust.edu.cn)
- Zhiwen Liu (993096281@qq.com)
- Yiwen Zhang (zhangyiwen@hust.edu.cn)