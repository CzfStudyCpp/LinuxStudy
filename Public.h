//一些通用，共用的头文件以及类型定义

#ifndef PUBLIC_H_
#define PUBLIC_H_

#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<string>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<sys/mman.h>
#include<unistd.h>
#include<stdlib.h>
#include<inttypes.h>

namespace linux_study{
	namespace largefile{
		const int32_t EXIT_DISK_OPER_INCOMPLETE =-8012;//读写的长度小于要求的长度
		const int32_t TFS_SUCCESS=0;
		const int32_t TFS_ERROR=-1;
	}
}

#endif