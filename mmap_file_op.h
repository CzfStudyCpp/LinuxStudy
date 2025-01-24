//文件映射+文件操作
#ifndef MMAP_LARGEFILE_MMAPFILE_OP_H
#define MMAP_LARGEFILE_MMAPFILE_OP_H

#include "Public.h"
#include "file_op.h"
#include "mmap_file.h"

namespace linux_study{
	namespace largefile{
		class MMapFileOperation:public FileOperation{
			public:
			  MMapFileOperation(const std::string&file_name_,const int open_flags_=O_CREAT|O_RDWR|O_LARGEFILE):
			  FileOperation(file_name_,open_flags_),map_file(NULL),is_mapped(false){
			  }
			  ~MMapFileOperation(){
				  if(map_file){
					  delete(map_file);
					  map_file=NULL;
				  }
				  
			  }
			  int pread_file(char *buf,const int32_t size,const int64_t offset);
			  int pwrite_file(const char *buf,const int32_t size,const int64_t offset);
			  int mmap_file(const MMapOption &mmap_option);//映射文件
			  int munmap_file();//解除映射
			  void *get_map_data()const;//获取数据
			  int flush_file();//刷新磁盘
			private:
			   MMapFile*map_file;
			   bool is_mapped;
		};
	}
}





#endif //MMAP_LARGEFILE_MMAPFILE_OP_H