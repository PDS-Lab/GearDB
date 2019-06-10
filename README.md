# GearDB
## 1 Introduction
This is the implementation of the paper "**GearDB: A GC-free Key-Value Store on HM-SMR Drives with Gear Compaction**" appeared in [FAST 2019](https://www.usenix.org/conference/fast19).

We implement GearDB based on [LevelDB](https://github.com/google/leveldb).




## 2 Compilation and Run
In this branch of GearDB, we use zonefs to replace the read and write interfaces in the main branch (i.e., Libzbc). Zonefs is a simple file system released by Western Digital.


“ Zonefs is a very simple file system exposing each zone of a zoned block device as a file. zonefs does not have any on-disk metadata and does not require a disk format before use. As such, zonefs does not require any userspace tool.”



### 2.1 Tools
You should Install the zonefs kernel module, then mount zonefs under a directory.


### 2.2 Compilation and Run 

1. Add the path of your HM-SMR drive or emulated HM-SMR drive (e.g. /mnt/seqwrite) in file "/GearDB/hm/hm_status.h":
```
static const char smr_filename[]="/mnt/seqwrite";
```
2. Set zone numbers and the number of the first sequential  zone according to your HM-SMR drive in file "/GearDB/hm/hm_manager.cc".  You may query the zonefs directory to get the information.
```
void HMManager::set_all_zonenum_and_first_zonenum(uint64_t *zonenum, uint64_t *first_zonenum) {
    *zonenum = 55356;
    *first_zonenum = 524;
    //To be improved
}
```
3. We have modified the Makefile to make the compilation of GearDB same with LevelDB. 

```
> make -j4
```
3. Now you may run and test GearDB with out-static/db_bench. The "out-static/libleveldb.a." is still available in GearDB.
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