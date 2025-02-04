#include "Public.h"
#include "index_handle.h"

const static int debug=1; 
namespace linux_study{
	
	namespace largefile{
		
		IndexHandle::IndexHandle(const std::string &base_path,const uint32_t main_block_id){
			std::stringstream tmp_stream;
			tmp_stream<<base_path<<INDEX_DIR_PREFIX<<main_block_id; 
			std::string index_path = tmp_stream.str();
			
			file_op =new MMapFileOperation(index_path,O_CREAT|O_RDWR|O_LARGEFILE);
			is_load=false;
		}
		IndexHandle::~IndexHandle(){
			
			if(file_op){
				delete(file_op);
				file_op=NULL;
			}
		}
		
		
		int IndexHandle::creat(const uint32_t logic_block_id,const int32_t bucket_size,const MMapOption map_option){
			//打印调试信息
			if(debug)printf("create index,block id:%u,bucket_size:%d,max_mmap_size:%d,first_mmap_size:%d,per_mmap_size:%d\n",
			logic_block_id,bucket_size,map_option.max_mmap_size_,map_option.first_mmap_size_,map_option.per_mmap_size_);
			
			//如果索引文件已经加载，则不需要创建
			if(is_load){
				fprintf(stderr,"Index creat failed,because it has been loaded.\n It's block id:%u,bucket_size:%d,max_mmap_size:%d,first_mmap_size:%d,per_mmap_size:%d\n",
						logic_block_id,bucket_size,map_option.max_mmap_size_,map_option.first_mmap_size_,map_option.per_mmap_size_);
				return linux_study::largefile::EXIT_INDEX_ALREADY_LOADED_ERROR;
			}
			
			if (bucket_size <= 0) {
				return linux_study::largefile::EXIT_BUCKET_CONFIGURE_ERROR; 
			}
			
			int ret =linux_study::largefile::TFS_SUCCESS;
			int64_t file_size=file_op->get_file_size();//获取已经创建的文件的大小
			
			//如果文件大小小于0，说明文件出错
			if(file_size<0){
				fprintf(stderr,"Index creat failed,because the get_file_size<0.\n It's block id:%u,bucket_size:%d,max_mmap_size:%d,first_mmap_size:%d,per_mmap_size:%d\n",
						logic_block_id,bucket_size,map_option.max_mmap_size_,map_option.first_mmap_size_,map_option.per_mmap_size_);
				return linux_study::largefile::TFS_ERROR;
			}else if(file_size==0){//empty file that need init，
				//init the blockInfo and index header
				IndexHeader indexHeader;
				indexHeader.block_info.block_id=logic_block_id;//assign the block id in the block info
				indexHeader.block_info.seq_no=1;  //the seq_no must start from 1;
				indexHeader.bucket_size=bucket_size;//the number of the bucket
				
				indexHeader.index_file_size=sizeof(IndexHeader)+bucket_size*sizeof(int32_t);//the offset=the indexHeader+the offset of the buckets
				
				//indexHeader+total buckets，
				char* init_data =new char[indexHeader.index_file_size];//数据信息缓冲
				memcpy(init_data,&indexHeader,sizeof(indexHeader));//将头部信息复制
				memset(init_data+sizeof(indexHeader),0,indexHeader.index_file_size-sizeof(indexHeader));//初始化设置头部之外的空间的信息为0
			
			    //write index header and buckets into index file
				ret=file_op->pwrite_file(init_data,indexHeader.index_file_size,0);
				
				delete [] init_data;
				init_data=nullptr;
				
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"Index creat failed,because it pwrite_file failed.\n It's block id:%u,bucket_size:%d,max_mmap_size:%d,first_mmap_size:%d,per_mmap_size:%d\n",
						logic_block_id,bucket_size,map_option.max_mmap_size_,map_option.first_mmap_size_,map_option.per_mmap_size_);
					return ret;
				}
				ret=file_op->flush_file();//将数据刷新回磁盘
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"Index creat failed,because it flushes the file failed.\n It's block id:%u,bucket_size:%d,max_mmap_size:%d,first_mmap_size:%d,per_mmap_size:%d\n",
						logic_block_id,bucket_size,map_option.max_mmap_size_,map_option.first_mmap_size_,map_option.per_mmap_size_);
				    return ret;
				}
			}else{ //the file is no empty no need creat
			fprintf(stderr,"Index creat failed,because the file is no empty.\n It's block id:%u,bucket_size:%d,max_mmap_size:%d,first_mmap_size:%d,per_mmap_size:%d\n",
						logic_block_id,bucket_size,map_option.max_mmap_size_,map_option.first_mmap_size_,map_option.per_mmap_size_);
				return linux_study::largefile::EXIT_META_UNEXPECT_FOUND_ERROR;
				
			}
			
			ret=file_op->mmap_file(map_option);//刷新回磁盘后，将文件映射到内存，方便后续使用
			if(ret!=linux_study::largefile::TFS_SUCCESS){
				fprintf(stderr,"Index creat failed,because it mmap_file failed.\n It's block id:%u,bucket_size:%d,max_mmap_size:%d,first_mmap_size:%d,per_mmap_size:%d\n",
						logic_block_id,bucket_size,map_option.max_mmap_size_,map_option.first_mmap_size_,map_option.per_mmap_size_);
				return ret;
			}
			//加载成功
			is_load=true;
			
			 // if(debug)printf("init block id:%d index successful. data file size:%d.index file size:%d,bucket size:%d,free head offset:%d ,seq_no:%d,size:%d,file count:%d,del_size:%d,del_file_count:%d,version:%d\n",
			                   // logic_block_id, indexHeader.data_file_offset, indexHeader.index_file_size, indexHeader.bucket_size, indexHeader.free_head_offset,
							   // indexHeader.block_info.seq_no, indexHeader.block_info.size, indexHeader.block_info.file_count, indexHeader.block_info.del_size,
							  // indexHeader.block_info.del_file_count, indexHeader.block_info.version);
							  
		     if(debug){
				 std::cout<<"the index header's pointer is:"<<index_header()<<std::endl;
				  printf("init block id:%d index successful. data file size:%d.index file size:%d,bucket size:%d,free head offset:%d ,seq_no:%d,size:%d,file count:%d,del_size:%d,del_file_count:%d,version:%d\n",
			                   logic_block_id, index_header()->data_file_offset, index_header()->index_file_size, 
							   index_header()->bucket_size,index_header()->free_head_offset,
							   blockInfo()->seq_no, blockInfo()->size, blockInfo()->file_count, blockInfo()->del_size,
							   blockInfo()->del_file_count, blockInfo()->version);
							  
			 }
			return linux_study::largefile::TFS_SUCCESS;
		}
		//加载索引块文件
		int IndexHandle::load(const uint32_t logic_block_id,const int32_t bucket_size,const MMapOption map_option){
			if(is_load){
			   return linux_study::largefile::EXIT_INDEX_ALREADY_LOADED_ERROR;	
			}
			if (bucket_size <= 0) {
				return linux_study::largefile::EXIT_BUCKET_CONFIGURE_ERROR; 
			}
			
			int ret=linux_study::largefile::TFS_SUCCESS;
			int64_t file_size=file_op->get_file_size();
			if(file_size<0){
				return file_size;
			}else if(file_size==0){//empty file that need init
			    return linux_study::largefile::EXIT_INDEX_CORRUPT_ERROR;
			}else{ //the file is no empty
			    //使用临时映射选项，预防一些可能存在修改映射选项的情况
				MMapOption tmp_map_option=map_option;
				//文件大小比初始映射选项的映射大小限制大，但是小于最大映射大小
				if(file_size>tmp_map_option.first_mmap_size_&&file_size<=tmp_map_option.max_mmap_size_){
					tmp_map_option.first_mmap_size_=file_size;
				}
				//将文件映射到内存，也就是加载数据的主要步骤
				ret=file_op->mmap_file(tmp_map_option);
				
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					return ret;
				}
				
				//成功了做一些合法性判断
				if(0==bucketSize()||0==blockInfo()->block_id){
					fprintf(stderr,"Index corrupt error. block id :%u,bucket size:%d\n",blockInfo()->block_id,bucketSize());
					return linux_study::largefile::EXIT_INDEX_CORRUPT_ERROR;
				}
				
				//还需要检查文件的大小,如果文件大小小于实际的索引文件大小，说明出现问题
				int32_t index_file_size_=sizeof(IndexHeader)+bucket_size*sizeof(int32_t);
				
				if(file_size<index_file_size_){
					fprintf(stderr,"Index corrupt error. block id :%u,bucket size:%d,file size:%ld,index file size:%d\n",
					blockInfo()->block_id,bucketSize(),file_size,index_file_size_);
					return linux_study::largefile::EXIT_INDEX_CORRUPT_ERROR;
				}
				
				//检查块id
				if(logic_block_id!=blockInfo()->block_id){
					fprintf(stderr,"Index block id conflict. Parameter block id :%d,memory block id:%d",logic_block_id,blockInfo()->block_id);
					return linux_study::largefile::EXIT_BLOCKID_CONFLICT_ERROR;
				}
				
				//check bucket size
				if(bucket_size!=bucketSize()){
					fprintf(stderr,"Index configure error. old bucket size:%d,new bucket size :%d",bucketSize(),bucket_size);
					return linux_study::largefile::EXIT_BUCKET_CONFIGURE_ERROR;
				}
				
				is_load=true;
				if(debug)printf("load block id:%d index successful. data file size:%d.index file size:%d,bucket size:%d,free head offset:%d ,seq_no:%d,size:%d,file count:%d,del_size:%d,del_file_count:%d,version:%d\n",
			                 logic_block_id, index_header()->data_file_offset, index_header()->index_file_size, 
							 index_header()->bucket_size,index_header()->free_head_offset,
							 blockInfo()->seq_no, blockInfo()->size, blockInfo()->file_count, blockInfo()->del_size,
							 blockInfo()->del_file_count, blockInfo()->version);
				return linux_study::largefile::TFS_SUCCESS;
				
			}
			
		}
		
		IndexHeader* IndexHandle::index_header(){
			return reinterpret_cast<IndexHeader*>(file_op->get_map_data());
			
		}
		BlockInfo* IndexHandle::blockInfo(){
			   //因为blockInfo变量的起始地址也是IndeHeader的起始地址
		       return reinterpret_cast<BlockInfo*>(file_op->get_map_data());
		}
		
		int32_t IndexHandle::bucketSize()const{
			return reinterpret_cast<IndexHeader*>(file_op->get_map_data())->bucket_size;
		}
					
	}
	
	
}