#include"mmap_file_op.h"
#include"Public.h"
#include <cinttypes> 
static int debug=1;

namespace linux_study{
	namespace largefile{
		
		int MMapFileOperation::mmap_file(const MMapOption &mmap_option){//初始化，将文件映射到内存
			if(mmap_option.max_mmap_size_<mmap_option.first_mmap_size_){
				return linux_study::largefile::TFS_ERROR;
			}
			if(mmap_option.max_mmap_size_<=0){
				return linux_study::largefile::TFS_ERROR;
			}
			int fd=check_file();
			if(fd<0){
				fprintf(stderr,"MMapFileOperation::mmap_file->check file failed");
				return linux_study::largefile::TFS_ERROR;
			}
			if(!is_mapped){
				if(map_file)delete(map_file);
				map_file=new MMapFile(mmap_option,fd);
				is_mapped=map_file->map_file(true);//映射文件
			}
			
			if(is_mapped)return linux_study::largefile::TFS_SUCCESS;
			else return linux_study::largefile::TFS_ERROR;
		
		}
		//解除映射
		int MMapFileOperation::munmap_file(){
			if(is_mapped&&map_file!=NULL){
				delete(map_file);
				is_mapped=false;
				return linux_study::largefile::TFS_SUCCESS;
			}
			return linux_study::largefile::TFS_ERROR;
		}
		void *MMapFileOperation::get_map_data()const{
			if(is_mapped&&map_file!=NULL){
				//printf("get map data successed\n");
				return map_file->get_data();
			}
		
			return NULL;
			
		}
		int MMapFileOperation::pread_file(char *buf,const int32_t size,const int64_t offset){
			//情况1：内存已经映射
			//情况1.1 内存已经映射的情况下但要读入的内容超出文件大小规模
			if(is_mapped&&(size+offset)>map_file->get_size()){
				//__PRI64_PREFIX是一个宏，通常用于处理 64 位整数的格式说明符，32位系统为lld，64位系统为ld
				if(debug) fprintf(stdout, "mmap_file_op_pread:size:%" PRId32 ", offset:%" PRId64 ", map file size:%d. need remap\n",
                    size,                // PRId32 对应 int32_t
                    offset,              // PRId64 对应 int64_t
                    map_file->get_size()); // 假设返回 int，用 %d
				//尝试一次扩容，一般情况下每次遇到超出规模都进行一次扩容就可以解决，如果存在一些情况需要，使得一次扩容不行，可以修改为进行多次扩容，直到符合大小或者达到最大容量限制
				map_file->remap_file();
			}
			//情况1.2：要读取的长度在文件大小规模内
			if(is_mapped&&(offset+size)<=map_file->get_size()){
				memcpy(buf,(char*)map_file->get_data()+offset,size);
				return linux_study::largefile::TFS_SUCCESS;
			}
			
			//情况二：内存没有映射或者映射不全
			return FileOperation::pread_file(buf,size,offset);
		}
		int MMapFileOperation::pwrite_file(const char *buf,const int32_t size,const int64_t offset){
			//情况一：已经映射的情况下，两种情况同读文件操作的应对情况相同
			if(is_mapped&&(size+offset)>map_file->get_size()){
				if(debug) fprintf(stdout, "mmap_file_op_pread:size:%" PRId32 ", offset:%" PRId64 ", map file size:%d. need remap\n",
                    size,                // PRId32 对应 int32_t
                    offset,              // PRId64 对应 int64_t
                    map_file->get_size()); // 假设返回 int，用 %d
				map_file->remap_file();
			}
			if(is_mapped&&(offset+size)<=map_file->get_size()){
				memcpy((char*)map_file->get_data()+offset,buf,size);
				return linux_study::largefile::TFS_SUCCESS;
			}
			
			//情况二：内存没有映射或者映射不全
			return FileOperation::pwrite_file(buf,size,offset);
			
		}
		//将文件同步持久化存储
		int MMapFileOperation::flush_file(){
			if(is_mapped){
				if(map_file->sync_file()){
					return linux_study::largefile::TFS_SUCCESS;
				}
				
				else{
                    fprintf(stderr,"the file is mapped but flush the file happend errors\n");
					return linux_study::largefile::TFS_ERROR;
				}
			}
			//没有映射但是有可能在内存中
			//fprintf(stderr,"the file is not mapped and flush the file \n");
			return FileOperation::flush_file();
		}
	}
}