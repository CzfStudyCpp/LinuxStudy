# Taobao Distributed File System Core Storage Engine

核心功能原理的学习实现

## 功能实现

### 公共配置

    1. Public.h(包括可能多次用到的头文件库，struct BlockInfo(块的元信息), MMapOption(映射文件的大小选项),MetaInfo(文件元信息哈希索引块))

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

    1. index_handle.h(include: struct IndexHeader)
    2. index_handle.cpp

## 功能测试

### 块初始化测试

    block_init_test.cpp

### 块加载程序

    block_load_stat.cpp

### 块写入测试程序

    block_write_test.cpp

### 块读取测试程序

    block_read_test.cpp

### 文件删除测试程序

    block_remove_test.cpp

### 主块数据整理

    block_space_cleanup_test.cpp

## 一键编译全部测试程序脚本

   One-Click_Compile_All_Programs.sh
   >>在 Windows 系统中通过 Samba 共享创建的 .sh 脚本文件时，文件中的换行符格式是 Windows 风格的 CRLF。这导致在 Unix/Linux 系统中运行脚本时出现问题，因为它们期望的换行符是 LF。

### solution

1. 安装 **dos2unix**：

    sudo apt-get install dos2unix

2. 转换脚本文件的换行符：

    dos2unix One-Click_Compile_All_Programs.sh
