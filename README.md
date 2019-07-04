# GearDB
## 1 Introduction
This is the implementation of the paper "**GearDB: A GC-free Key-Value Store on HM-SMR Drives with Gear Compaction**" appeared in [FAST 2019](https://www.usenix.org/conference/fast19).

We implement GearDB based on [LevelDB](https://github.com/google/leveldb).




## 2 Compilation and Run
### 2.1 Tools
To access HM-SMR drives, GearDB uses the [**Libzbc**](https://github.com/hgst/libzbc) library which provides functions to manipulate disks supporting ZBC and ZAC commands.  To run GearDB, please download and install [**Libzbc**](https://github.com/hgst/libzbc) first:
```
> git clone https://github.com/hgst/libzbc  
> sh ./autogen.sh
> ./configure
> make
> make install    //necessary
```

### 2.2 Compilation and Run 
1. If you use an HM-SMR drive (Host-managed shingled magnetic recording drive) as your storage medium, **skip this step. **If you do not have one, emulate an HM-SMR drive with your conventional HDD as following :   
```
> cd libzbc
> zbc_set_zones /dev/sdb1 set_sz 256 256  //Block device /dev/sdb1 is an example
```
2. Add the path of your HM-SMR drive or emulated HM-SMR drive (e.g., /dev/sdb1) in file "/GearDB/hm/hm_status.h":
```
static const char smr_filename[]="/dev/sdb1";
```
3.  Get the logic block size and physical block size of your HM-SMR drive. Set their sizes in file “/GearDB/hm/hm_status.h”. This is because write operations on HM-SMR drives should align with the PHYSICAL_BLOCK_SIZE and read operations on HM-SMR drives should align with the LOGICAL_BLOCK_SIZE.
```
#define PHYSICAL_BLOCK_SIZE 4096   
#define LOGICAL_BLOCK_SIZE 4096
```
4. We have modified the Makefile to make the compilation of GearDB same with LevelDB. 

```
> make -j4
```
5. Now you may run and test GearDB with out-static/db_bench. The "out-static/libleveldb.a." is still available in GearDB.
```
> ./out-static/db_bench
```
## 3 GearDB's differences with [LevelDB](https://github.com/google/leveldb)
To get the source code of LevelDB in the same edition used by GearDB,  do the following steps:

```
> git clone https://github.com/google/leveldb
> cd leveldb
> git checkout  23162ca1c6d891a9c5fe0e0fab1193cd54ed1b4f
```
Or, you may download it at : <https://github.com/google/leveldb/tree/23162ca1c6d891a9c5fe0e0fab1193cd54ed1b4f> 

Since GearDB is implemented based on LevelDB, here we list the main differences between GearDB and LevelDB.

1. Added file: hm/*。
2. Modified file:  

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
We thank [LevelDB](https://github.com/google/leveldb) and [Libzbc](https://github.com/hgst/libzbc)! 

We used them to complete our implementation.



## 5 Attentions
This version does not support reloading databases, which means it will get an error if you open the database again after the program finished. The version supported reloading is implemented. However, it makes the operation of GearDB a little bit cumbersome. If you test GearDB with the macro-benchmark [YCSB](https://github.com/brianfrankcooper/YCSB.git), the reload function will be needed. You may contact *993096281@qq.com* to get the support for reloading a database. 



## 6 Contributors
- Ting Yao ([tingyao@hust.edu.cn](mailto:tingyao@hust.edu.cn) )
- Zhiwen Liu (993096281@qq.com)
- Yiwen Zhang (zhangyiwen@hust.edu.cn)