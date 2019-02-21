# GearDB
## 1 介绍
GearDB是论文《GearDB: A GC-free Key-Value Store on HM-SMR Drives with Gear Compaction》的实现代码，基于[LevelDB](https://github.com/google/leveldb)进行修改。


## 2 编译与运行
### 2.1 工具
GearDB 需要 [**Libzbc**](https://github.com/hgst/libzbc) (<https://github.com/hgst/libzbc>) 工具来替换磁盘的读写接口，所以必须先下载和安装[**Libzbc**](https://github.com/hgst/libzbc)。  
[**Libzbc**](https://github.com/hgst/libzbc)的安装步骤：
```
> git clone https://github.com/hgst/libzbc  
> sh ./autogen.sh
> ./configure
> make
> make install    //不可缺少
```

### 2.2 编译和运行步骤
1. 如果你用瓦记录磁盘当作存储介质，你不用管这步；如果你是通过普通硬盘来模拟主机管理瓦记录磁盘，你需要利用libzbc来进行模拟操作：  
```
> cd libzbc
> zbc_set_zones /dev/sdb1 set_sz 256 256  //块设备/dev/sdb1是一个例子
```
2. 选择好主机管理瓦记录磁盘的路径，例如使用/dev/sdb1,你需要在/GearDB/hm/hm_status.h文件中添加路径：
```
static const char smr_filename[]="/dev/sdb1";
```
3. GearDB的编译和leveldb一样，我们修改了Makefile的一点内容：
```
> make -j4
```
4. 然后可以使用out-static/db_bench来进行测试，或者通过静态链接库out-static/libleveldb.a做更多的事情。
```
> ./out-static/db_bench
```

## 3 与[LevelDB](https://github.com/google/leveldb)的不同
我们基于LevelDB进行修改，如果想获取源版本，可执行下面的操作：
```
> git clone https://github.com/google/leveldb
> cd leveldb
> git checkout  23162ca1c6d891a9c5fe0e0fab1193cd54ed1b4f
```
另一种方法，你可以打开链接：<https://github.com/google/leveldb/tree/23162ca1c6d891a9c5fe0e0fab1193cd54ed1b4f>  
然后下载。 

1. 增加的文件：hm/*。
2. 修改的文件：  
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
## 4 致谢
感谢[LevelDB](https://github.com/google/leveldb)和[Libzbc](https://github.com/hgst/libzbc)！我们使用了它们来完成自己的实验。
## 5 注意事项
这个版本不支持数据库的恢复功能。这意味着如果程序运行结束后，再打开存在的数据库，它会出现错误。我们实现了恢复功能，但是它会使操作过程变得麻烦一些，所以没有给出。例如在[YCSB](https://github.com/brianfrankcooper/YCSB.git)中进行测试时，它需要数据库的恢复功能，更加重要的是，它的操作流程比较繁琐，如果你需要支持数据库恢复的版本，你可以联系*993096281@qq.com*。

## 6 贡献者
- 姚婷 (tingyao@hust.edu.cn)
- 刘志文 (993096281@qq.com)
- 张艺文 (zhangyiwen@hust.edu.cn)