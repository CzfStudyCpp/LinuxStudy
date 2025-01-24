
#include"file_op.h"
#include"Public.h"

namespace linux_study{
	namespace largefile{
	// file_op.cpp
	FileOperation::FileOperation(const std::string &file_name_, const int open_flags_) :
		fd(-1), open_flags(open_flags_) {
		file_name = strdup(file_name_.c_str()); // 创建一块新的内存复制文件名，然后再传给成员变量
	}

		FileOperation::~FileOperation(){
			if(fd>0)
			 ::close(fd);//关闭文件描述符
		      //带上两个冒号，表明这个函数是全局的，那么就会直接调用库里的函数
			
			if(file_name!=NULL){
				free(file_name);
				file_name=NULL;
			}
			
		}
		int FileOperation::open_file(){
			if(fd>0){
				//已经开启了就先关闭
				close(fd);
				fd=-1;
			}
			fd= ::open(file_name,open_flags,OPEN_MODE);//全局的函数
			if(fd<0){
				return errno;
			}
			return fd;
		}
		bool FileOperation::close_file(){
			if(fd<0){
			return false;
			}
			
			close(fd);
			fd=-1;
			return true;
		}
		int64_t FileOperation::get_file_size(){
			//当没有调用打开文件函数之前，也可以获取
			int fd=check_file();
			if(fd<0){
				return -1;
			}
			struct stat statbuf;
			
			//由fd取得文件状态，stat是文件状态的元信息结构体，
			if(fstat(fd,&statbuf)!=0){//将fd的值复制到statbuf中，成功则返回0，失败返回-1
				return -1;
			}
			
			return statbuf.st_size;
			
			
		}
		//改变文件大小
		int FileOperation::ftruncate_file(const int64_t length){
			int fd=check_file();
			if(fd<0){
				return fd;
			}
			return ftruncate(fd,length);//修改指定的文件的大小，改为参数length指定的大小
		}
			
		
		int FileOperation::check_file(){
			if(fd<0){
				fd=open_file();
			}
			return fd;
				
		}
		
		int FileOperation::seek_file(const int64_t offset){
			int fd=check_file();
			if(fd<0)return fd;
			return lseek(fd,offset,SEEK_SET);
			//SEEK_SET 参数 offset 即为新的读写位置。
			// SEEK_CUR 以目前的读写位置往后增加 offset 个位移量。
			// SEEK_END 将读写位置指向文件尾后再增加 offset 个位移量。
		}
		
		int FileOperation::flush_file(){
			//检查是否同步，如果文件是同步状态打开，那么它的操作都是同步写回磁盘的，那么就不需要刷回磁盘操作
			if(open_flags&&O_SYNC){
				return 0;
			}
			int fd=check_file();
			if(fd<0)return fd;
			
			return fsync(fd);
		}
		
		int FileOperation::unlink_file(){
			close_file();
			return unlink(file_name);
			//删除文件
		}
		
		int FileOperation::pread_file(char *buf,const int32_t nbytes,const int64_t offset){
			int32_t left=nbytes;
			int64_t read_offset =offset;
			int32_t read_len =0;
			char *p_tmp =buf;
			int i=0;
			while(left>0){
				++i;
				if(i>=MAX_DISKTIMES){
				    break;
				 }
				 if(check_file()<0){
					 return -errno;
				 }
				 read_len =pread64(fd,p_tmp,left,read_offset);
				 if(read_len<0){
					  //errno是全局变量，是会变化的，如果是多线程处理。那么可能会修改
					 read_len=-errno;
					//EINTR        4  /* Interrupted system call */
					//EAGAIN      11  /* Try again */
					 if(-read_len == EINTR||EAGAIN == -read_len)
						 continue;
				     //#define EBADF        9  /* Bad file number */
				     else if(EBADF == -read_len){
					  fd=-1;
					 return read_len;
				     }
				     else return read_len;
				 }
				 //文件读到尾部为0
				 else if(0 ==read_len)break;
				 
				 left -=read_len;
				 p_tmp +=read_len;
				 read_offset += read_len;
			}
			if(0!=left){
				return linux_study::largefile::EXIT_DISK_OPER_INCOMPLETE;
			}
			return linux_study::largefile::TFS_SUCCESS;
		}		 
		int FileOperation::pwrite_file(const char *buf,const int32_t nbytes,const int64_t offset){
            int32_t left=nbytes;
			int64_t write_offset =offset;
			int32_t write_len =0;
			const char *p_tmp =buf;
			int i=0;
			while(left>0){
				++i;
				if(i>=MAX_DISKTIMES){
				    break;
				 }
				 if(check_file()<0){
					 return -errno;
				 }
				write_len =pwrite64(fd,p_tmp,left,write_offset);
				 if(write_len<0){
					 write_len=-errno;
					 //errno是全局变量，是会变化的，如果是多线程处理。那么可能会修改
					 if(-write_len == EINTR||EAGAIN == -write_len)
						 continue;
				 
				     else if(EBADF == -write_len){
					  fd=-1;
					  continue;
				     }
				     else return write_len;
				 }
				 else if(0 == write_len)break;
				 
				 left -=write_len;
				 p_tmp +=write_len;
				 write_offset += write_len;
			}
			if(0!=left){
				return linux_study::largefile::EXIT_DISK_OPER_INCOMPLETE;
			}
			return linux_study::largefile::TFS_SUCCESS;

		}

        int FileOperation::write_file(const char*buf,const int32_t nbytes){		
			 int32_t left=nbytes;
			int32_t write_len =0;
			const char *p_tmp =buf;
			int i=0;
			while(left>0){
				++i;
				if(i>=MAX_DISKTIMES){
				    break;
				 }
				 if(check_file()<0){
					 return -errno;
				 }
				 write_len=::write(fd,p_tmp,left);
				 
				 if(write_len<0){
					 write_len=-errno;
					 //errno是全局变量，是会变化的，如果是多线程处理。那么可能会修改
					 if(-write_len == EINTR||EAGAIN == -write_len)
						 continue;
				 
				     else if(EBADF == -write_len){
						  fd=-1;
						continue;//另外一种方式，可以尝试多采用一种方法，多尝试一次
				     }
				     else return write_len;
				 }
				 else if(0 == write_len)break;
				 
				 left -=write_len;
				 p_tmp +=write_len;
			}
			if(0!=left){
				return linux_study::largefile::EXIT_DISK_OPER_INCOMPLETE;
			}
			return linux_study::largefile::TFS_SUCCESS;
		}
			
		
	}
}