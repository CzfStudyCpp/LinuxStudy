#ifndef LARGEFILE_INDEX_HANDLE_H_
#define LARGEFILE_INDEX_HANDLE_H_

#include"Public.h"
#include"mmap_file_op.h"
namespace linux_study{
	
	namespace largefile{

		//索引的头部信息
		struct IndexHeader{
		    public:
			IndexHeader(){
				memset(this,0,sizeof(IndexHeader));
			}
			BlockInfo block_info;//块信息
			
			//脏数据标记暂时不用
			
			int32_t bucket_size;//the size of the hash bucket
			int32_t data_file_offset;//offset to write next data in block_info（the data length）
			int32_t index_file_size;//the current offset of the index file. offset after:index_header+all buckets (the length of the index file)
			int32_t free_head_offset;//the meta node list that can be reuse
			
             		
		};
		//the class to handle the Index
		class IndexHandle{
		      public:
			    IndexHandle(const std::string &base_path,const uint32_t main_block_id);
				~IndexHandle();
				//创建索引文件
				int creat(const uint32_t logic_block_id,const int32_t bucket_size,const MMapOption map_option);
				//加载索引文件
				int load(const uint32_t logic_block_id,const int32_t bucket_size,const MMapOption map_option);
				//从映射的文件内存中获取IndexHeader数据地址
				IndexHeader*index_header();
				//获取头部索引中的块信息的指针
				BlockInfo * blockInfo();
				//get the hash bucket size
				int32_t bucketSize()const;
			  private:
			    //the main p[erator
			    MMapFileOperation *file_op;
				bool is_load;
		
		};
		
	}
	
	
}



#endif  //LARGEFILE_INDEX_HANDLE_H_