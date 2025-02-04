# The hash map file management and store
文件通过哈希方式的存储和管理引擎
## 功能实现

### 公共配置
    1. Public.h(包括可能多次用到的头文件库，struct BlockInfo(块的元信息),MMapOption(映射文件的大小选项),MetaInfo(文件元信息哈希索引块))
### 文件的映射操作，将文件映射到内存的实现
    1. mmap_file.h
    2. mmap_file.cpp

### 文件的基础操作的实现
    1. file_op.h
    2. file_op.cpp
### 映射到内存中的文件操作实现
    1. mmap_file_op.h
    2. mmap_file_op.cpp
### 索引文件的处理类
    1. index_handle.h(包括struct IndexHeader)
    2. index_handle.cpp
