#ifndef FILE_OP_H_
#define FILE_OP_H_

#include"Public.h"

namespace linux_study
{
	namespace largefile
	{
		class FileOperation{
			public:
			FileOperation(const std::string &file_name,const int open_flags=O_RDWR|O_LARGEFILE);
			~FileOperation();
			
			int open_file();
			bool close_file();
			int flush_file();//写入硬盘，将内存缓冲区数据写入磁盘
			
			int unlink_file();//删除文件
			
			//考虑设计为虚函数，这样继承之后，父类和子类指针可以调用自己相应的函数
			//精细化读
			virtual int pread_file(char *buf,const int32_t nbytes,const int64_t offset);//读文件
			//精细化写
			virtual int pwrite_file(const char *buf,const int32_t nbytes,const int64_t offset);//写文件
			//直接在当前位置开始写
			int write_file(const char*buf,const int32_t nbytes);//直接写文件
			
			int64_t get_file_size();//获取大小
			int ftruncate_file(const int64_t length);//截断文件
			int seek_file(const int64_t offset);//移动文件指针到指定位置
			
			int get_fd()const;//获取文件句柄
			
			protected:
				int fd;//文件句柄，
				int open_flags;
				char *file_name;
			
			protected:
				//用户本身拥有读写权力，其他用户只有读取
				static const mode_t OPEN_MODE=0644;
				//最大的磁盘读取次数
				static const int MAX_DISKTIMES=5;
			
			protected:
			    //当文件没有打开的时候，但是为了获取信息必须打开
				int check_file();
			
			
		};
		
	}
}
/*
标准输入（stdin）：文件描述符 0，通常用于接收用户输入。
标准输出（stdout）：文件描述符 1，通常用于程序的正常输出。
标准错误（stderr）：文件描述符 2，通常用于程序的错误输出。
*/
#endif     