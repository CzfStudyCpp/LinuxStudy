
#include"file_op.h"
#include"Public.h"

const static int debug=1;
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
				fprintf(stderr,"the fd for the file is NULL with the fd :%d\n",fd);
				return -1;
			}
			struct stat statbuf;
			
			//由fd取得文件状态，stat是文件状态的元信息结构体，
			if(fstat(fd,&statbuf)!=0){//将fd的值复制到statbuf中，成功则返回0，失败返回-1
			    fprintf(stderr,"get the file stat failed with the fd :%d\n",fd);
				return -1;
			}
			printf("get the file stat is success with the fd : %d,and the stat.size is : %ld\n",fd,statbuf.st_size);
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
				if(debug)fprintf(stderr,"the flushing file the open is true and the O_SYNC is opened\n");
				return linux_study::largefile::TFS_SUCCESS;
			}
			int fd=check_file();
			if(fd<0){
				if(debug)fprintf(stderr,"the FileOperation::flush_file  the fd is <0:the fd is :%d\n",fd);
				return fd;
			}
            if(debug)fprintf(stderr,"the FileOperation::flush_file,the function fsync() runs and the fd is >=0:the fd is :%d\n",fd);			
			return fsync(fd);
			
			// 使用fsync()系统调用，强制将文件描述符关联的所有数据（含元数据）同步写入磁盘。

			// 同步阻塞操作，等待磁盘写入完成才返回。

			// 适用场景：常规文件I/O操作的持久化保证。
		}
		
		int FileOperation::unlink_file(){
			close_file();
			int ret=unlink(file_name);
			if(ret!=0){
				fprintf(stderr,"remove the file fail with the ret is :%d,and the error desc is :%s\n",ret,strerror(errno));
				return linux_study::largefile::TFS_ERROR;
			}
			else{
				return linux_study::largefile::TFS_SUCCESS;
			}
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
				//超出限制的磁盘访问次数
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
		int32_t FileOperation::copy_main_block(FileOperation * old_main_block,std::vector<MetaInfo>& usefulMetaList){
			int32_t file_id_=1;
			int32_t data_file_offset=0;
			int32_t ret=linux_study::largefile::TFS_SUCCESS;
			
			for(auto &meta:usefulMetaList){
				int32_t now_data_size=meta.get_size();
				char buffer[now_data_size+1];
				buffer[now_data_size]='\0';
				
				//从对应的数据偏移读取，承载数据的buffer，文件大小规模size，对应文件在旧数据块中的偏移量
				ret=old_main_block->pread_file(buffer,now_data_size,meta.get_offset());
				if (ret != linux_study::largefile::TFS_SUCCESS) {
				fprintf(stderr, "in copy_main_block, old_main_block read the data fail, the data_size is :%d, the old data_offset is: %d with the ret: %d\n",
						now_data_size, meta.get_offset(), ret);
				return ret;
				}

				
				//写入新块
				ret=this->pwrite_file(buffer,now_data_size,data_file_offset);
				if (ret != linux_study::largefile::TFS_SUCCESS) {
					fprintf(stderr, "in copy_main_block, old_main_block read the data fail, the data_size is :%d, the old data_offset is: %d with the ret: %d\n",
							now_data_size, meta.get_offset(), ret);
					return ret;
				}

				//更新元信息，数据主块相关的元数据信息是文件的id，数据的偏移量，以及数据大小，在这里数据大小不用修改
				meta.set_file_id(file_id_++);
				meta.set_offset(data_file_offset);
				data_file_offset+=now_data_size;
					
			}
			if(debug)printf("copy the Block with the file_name :%s,the file_count is : %d,the data_file_offset :%d\n"
			,file_name,file_id_-1,data_file_offset);
			flush_file();
			return linux_study::largefile::TFS_SUCCESS;
			
		}
		int32_t FileOperation::batch_clean_up(std::vector<MetaInfo>&usefulMetaList){
			int32_t file_id_=1;
			int32_t data_file_offset=0;
			int32_t ret=linux_study::largefile::TFS_SUCCESS;
			for(auto &meta:usefulMetaList){
				int32_t now_data_size=meta.get_size();
                if(meta.get_offset()==0){
                  //第一个节点，不需要设置meta
				  file_id_++;
				  data_file_offset+=now_data_size;
				  continue;
				}				
                char buffer[now_data_size+1];
				buffer[now_data_size]='\0';	
                ret=this->pread_file(buffer,now_data_size,meta.get_offset());
                if(ret!=linux_study::largefile::TFS_SUCCESS){
				   fprintf(stderr,"in batch_clean_up pread_file fail!the data_file_offset :%d,data_size:%d, with the ret: %d\n"
				   ,meta.get_offset(),now_data_size,ret);
				   return ret;
				}
                ret=this->pwrite_file(buffer,now_data_size,data_file_offset);
				if (ret != linux_study::largefile::TFS_SUCCESS) {
					fprintf(stderr, "in batch_clean_up, pwrite_file fail, the data_size is :%d, the data_offset is: %d with the ret: %d\n",
							now_data_size,data_file_offset,ret);
					return ret;
				}
                meta.set_file_id(file_id_++);
                meta.set_offset(data_file_offset);
				data_file_offset+=now_data_size;				
			}
			if(debug)printf("batch_clean_up Block with the file_name :%s,the file_count is : %d,the data_file_offset :%d\n"
			,file_name,file_id_-1,data_file_offset);
			flush_file();
			return linux_study::largefile::TFS_SUCCESS;
		}
		int FileOperation::rename_file(const std::string &old_path, const std::string &new_path) {
			if (old_path.empty()) {
				fprintf(stderr, "in rename the old_path is empty\n");
				return linux_study::largefile::FILE_NAME_EMPTY_ERROR;
			}
			if (new_path.empty()) {
				fprintf(stderr, "in rename the new_path is empty\n");
				return linux_study::largefile::FILE_NAME_EMPTY_ERROR;
			}
			   printf("Renaming file from: %s to: %s\n", new_path.c_str(), old_path.c_str());
			int ret = std::rename(new_path.c_str(),old_path.c_str());
			if (ret != 0) {
				fprintf(stderr, "rename the file fail with the ret: %d, the error desc is: %s\n", ret, strerror(errno));
				return linux_study::largefile::RENAME_FILE_ERROR;
			}
			else{
				printf("rename the new block successfully!\n");
			}
			if (file_name) {
				free(file_name); // 释放旧的文件名内存
				file_name = nullptr; // 防止悬空指针
			}
			file_name = strdup(new_path.c_str()); // 复制新的文件名到成员变量
			printf("change the member file_name of the new block successfully!\n");
			return linux_study::largefile::TFS_SUCCESS;
		}

		
	}
}