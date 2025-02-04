
#ifndef LINUX_HEADFILE
#define LINUX_HEADFILE
#include<stdint.h>
#include<unistd.h>
#include"Public.h"
namespace linux_study
{
		namespace largefile
		{

				
			class MMapFile
			{
			public:
				MMapFile();
				explicit MMapFile(const int fd);//避免一个带参的构造函数因为等号赋值带来的歧义
				MMapFile(const MMapOption& mmap_option, const int fd);
				~MMapFile();

				bool sync_file();//同步文件
				bool map_file(const bool write = false);//将文件映射到内存，同时设置访问权限（默认不可写）
				void* get_data()const;//获取映射到内存的数据的首地址
				int32_t get_size()const;//获取要映射数据的大小

				bool munmap_file();//解除映射
				//用于追加存储空间
				bool remap_file();//重新映射

			private:
				bool ensure_file_size(const int32_t size);//内存扩容


			private:
				int32_t size;
				int fd;//默认情况下大于等于0
				void* data;
				struct MMapOption mmap_file_option_;

			};
				
				
		}
}



#endif