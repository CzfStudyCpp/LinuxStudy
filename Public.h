//一些通用，共用的头文件以及类型定义

#ifndef PUBLIC_H_
#define PUBLIC_H_
#include<sstream>
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

#include <cassert>
namespace linux_study{
	namespace largefile{
		
		const int32_t EXIT_DISK_OPER_INCOMPLETE =-8012;//读写的长度小于要求的长度
		const int32_t TFS_SUCCESS=1;//系统文件操作成功
		const int32_t TFS_ERROR=-1;
		const int32_t EXIT_INDEX_ALREADY_LOADED_ERROR=-80;//index file is loaded when create or loading
		const int32_t EXIT_META_UNEXPECT_FOUND_ERROR=-8014;//the meta found in the index when insert
		const int32_t EXIT_INDEX_CORRUPT_ERROR=-8015;//the index file is corrupt;
		const int32_t EXIT_BLOCKID_CONFLICT_ERROR=-8016; //the index block id is conflict
		const int32_t EXIT_BUCKET_CONFIGURE_ERROR=-8017; //the bucket size error
		
		//映射选项
		struct MMapOption
		{
			int32_t max_mmap_size_=0;
			int32_t first_mmap_size_=0;
			int32_t per_mmap_size_=0;

		};
		
		//索引文件和索引块的信息
		static const std::string MAINBLOCK_DIR_PREFIX="/mainblock/";
		static const std::string INDEX_DIR_PREFIX="/index/";
		static const mode_t DIR_MODE=0755;
		
		//块的元信息
		struct BlockInfo{
			 uint32_t block_id;  //块的编号，TFS=NameServer+DataServer
			 int32_t  version;   //版本号
			 int32_t  file_count;  //当前已保存文件总数
			 int32_t  size;        //文件数据总大小
			 int32_t  del_file_count;  //已删除的文件数量
			 int32_t  del_size;        //已删除的文件数据总大小
			 uint32_t seq_no;          //下一个可分配的文件编号 1...2^64-1，为下一个需要的块分配编号
		
			BlockInfo(){
				memset(this,0,sizeof(BlockInfo));
			}
			inline bool operator==(const BlockInfo & rhs)const {
				return block_id == rhs.block_id && version == rhs.version && file_count == rhs.file_count && size == rhs.size
				       && del_file_count == rhs.del_file_count && del_size == rhs.del_size && seq_no == rhs.seq_no;
			}
		};
		//文件哈希索引块
		struct MetaInfo{
			public:
			  MetaInfo(){
				  init();
			  }
			  MetaInfo(const uint64_t file_id_,const int32_t in_offset,const int32_t file_size,const int32_t next_meta_offset_){
				  file_id=file_id_;
				  location.inner_offset=in_offset;
				  location.size=file_size;
				  next_meta_offset=next_meta_offset_;
			  }
			  MetaInfo(const MetaInfo& meta_info){
				  memcpy(this,&meta_info,sizeof(MetaInfo));
			  }
			  
			  MetaInfo &operator=(const MetaInfo & meta_info){
				  if(this==&meta_info)return *this;
				  
				  file_id=meta_info.file_id;
				  location.inner_offset=meta_info.location.inner_offset;
				  location.size=meta_info.location.size;
				  next_meta_offset=meta_info.next_meta_offset;
				  return *this;
			  }
			  
			  MetaInfo & clone(const MetaInfo& meta_info){
				  assert(this!= &meta_info);
				  
				  file_id=meta_info.file_id;
				  location.inner_offset=meta_info.location.inner_offset;
				  location.size=meta_info.location.size;
				  next_meta_offset=meta_info.next_meta_offset;
				  return *this;
			  }
			  bool operator==(const MetaInfo&meta_info)const{
				  return file_id==meta_info.file_id &&
				         location.inner_offset==meta_info.location.inner_offset &&
				         location.size==meta_info.location.size &&
				         next_meta_offset==meta_info.next_meta_offset;
			  }
			  uint64_t get_key()const{
				 return file_id;
			  }
			  void set_key(const uint64_t key){
				 file_id=key;
			  }
			  
			  uint64_t get_file_id()const{
				  return file_id;
			  }
			  
			  void set_file_id(const uint64_t file_id_){
				 file_id=file_id_;
			  }
			  
			  int32_t get_offset()const{
				  return location.inner_offset;
			  }
			  void set_offset(const int32_t offset){
				  location.inner_offset=offset;
			  }
			  int32_t get_size()const{
				  return location.size;
			  }
			  void set_size(const int32_t file_size){
				  location.size=file_size;
			  }
			  int32_t get_next_meta_offset()const{
				  return next_meta_offset;
			  }
			  void set_next_meta_offset(const int32_t offset){
				  next_meta_offset=offset;
			  }
			private:
				uint64_t file_id;
			
				struct{
					int32_t inner_offset;
					int32_t size;
				}location;
				
				int32_t next_meta_offset;
			
			private:
			   void init(){
				   file_id=0;
				   location.inner_offset=0;
				   location.size=0;
				   next_meta_offset=0;
			   }
		};
	}
}

#endif  // PUBLIC_H_  //