#include "mmap_file.h"
#include"Public.h"
static bool debug=true;
namespace linux_study
{
	namespace largefile
	{
		MMapFile::MMapFile() :
			size(0), fd(-1), data(NULL) {};
		
		MMapFile::MMapFile(const int _fd_) :
			size(0), fd(_fd_), data(NULL) {};
		
		
		MMapFile::MMapFile(const MMapOption& mmap_option, const int _fd_) :
			size(0), fd(_fd_), data(NULL)
		{
			mmap_file_option_.max_mmap_size_ = mmap_option.max_mmap_size_;
			mmap_file_option_.first_mmap_size_ = mmap_option.first_mmap_size_;
			mmap_file_option_.per_mmap_size_ = mmap_option.per_mmap_size_;
		};
		
		MMapFile::~MMapFile()
		{
			//如果data不为空，说明已经有读写操作
			if (data) {
				//设置一个测试开关是一个好的习惯
				if (debug)printf("mmap file destruct,fd:%d,maped size:%d,data:%p\n", fd, size, data);
				//同步内存数据
				msync(data, size, MS_SYNC);
				//解除映射
				munmap(data, size);
				size = 0;
				data = nullptr;
				fd = -1;

				mmap_file_option_.max_mmap_size_ = 0;
				mmap_file_option_.first_mmap_size_ = 0;
				mmap_file_option_.per_mmap_size_ = 0;
			}
		}
		
		bool MMapFile::sync_file()
		{
			//同步文件，这里采用异步方案，效率更高
			if(data!=nullptr&&size>=0){
				return msync(data,size,MS_ASYNC)==0;
			}
			//没有同步，则不需要执行，也相当于同步了
			return true;
		
		}
		
		//文件映射
		bool MMapFile::map_file(const bool write){
			int flags=PROT_READ;
			if(write){
				flags|=PROT_WRITE;
			}
			if(fd<0)return false;
			if(0==mmap_file_option_.max_mmap_size_)return false;
			
			if(size<mmap_file_option_.max_mmap_size_)size=mmap_file_option_.first_mmap_size_;
			else size=mmap_file_option_.max_mmap_size_;
			
			if(!ensure_file_size(size)){
				fprintf(stderr,"ensure file failed int map_file,size:%d\n",size);
				return false;
			}
			
			
			data=mmap(0,size,flags,MAP_SHARED,fd,0);
			
			if(MAP_FAILED==data){
				fprintf(stderr,"map file failed:%s\n",strerror(errno));
				size=0;
				fd=-1;
				data=nullptr;
				return false;
			}
			if(debug){
				printf("mmap file successedd,fd:%d maped size:%d,data:%p\n",fd,size,data);

			}
			return true;
			}
		void* MMapFile::get_data()const
			{
				return data;
			}
			
		int32_t  MMapFile::get_size()const
			{
				return size;
			}
			
		bool MMapFile::munmap_file()//解除映射
			{
				if(munmap(data,size)==0)return true;
				else return false;
			}
				
		bool MMapFile::remap_file()//重新映射
			{
				//重映射之前没有进行
				if(fd<0||data==nullptr){
					fprintf(stderr,"mremap not mapped yet\n");
					return false;
				}
				//不能超过规定的文件的最大大小
				if(size>=mmap_file_option_.max_mmap_size_){
					fprintf(stderr,"reach the max size now size:%d,max size:%d\n",size,mmap_file_option_.max_mmap_size_);
				}
				int32_t new_size=size+mmap_file_option_.per_mmap_size_;
				if(new_size>mmap_file_option_.max_mmap_size_){
					new_size=mmap_file_option_.max_mmap_size_;
				}
				if(!ensure_file_size(new_size)){
					fprintf(stderr,"ensure file failed in remap_file,size:%d\n",size);
					return false;
				}
				if(debug)printf("mremap start fd:%d,noew size :%d,new_size:%d,,old data:%p\n",fd,size,new_size,data);
				//重新定位分配新的空间
				void *new_map_data=mremap(data,size,new_size,MREMAP_MAYMOVE);
				if(MAP_FAILED==new_map_data){
					fprintf(stderr,"mremap failed ,new size:%d,error desc:%s\n",new_size,strerror(errno));
					return false;
				}
				else
					printf("mremap start fd:%d,new size :%d,new_size:%d,,old data:%p\n",fd,size,new_size,data);
				data=new_map_data;
				size =new_size;
				return true;
			}
			
			//调整大小
			bool MMapFile::ensure_file_size(const int32_t size){
					struct stat s;//存放文件状态的
					//fstat获取文件状态，C++提供的API
					if(fstat(fd,&s)<0){//如果内存大小小于0，那么文件出错
						fprintf(stderr,"fstat error ,error dese:%s\n",strerror(errno));
						return false;
					}
					if(s.st_size<size){
						//ftruncate调整文件大小，C++提供的API函数，小于0表示出错
						if(ftruncate(fd,size)<0){
							fprintf(stderr,"ftruncate error,size:%d,error desc:%s\n",size,strerror(errno));
							return false;
						}
					}
					return true;
						
			}
			
		
	}
}	