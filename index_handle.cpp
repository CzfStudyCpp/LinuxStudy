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
				
				//init the offset is the start
				indexHeader.index_file_offset=sizeof(IndexHeader)+bucket_size*sizeof(int32_t);//the offset=the indexHeader+the offset of the buckets
				
				//indexHeader+total buckets，
				char* init_data =new char[indexHeader.index_file_offset];//数据信息缓冲
				memcpy(init_data,&indexHeader,sizeof(indexHeader));//将头部信息复制
				memset(init_data+sizeof(indexHeader),0,indexHeader.index_file_offset-sizeof(indexHeader));//初始化设置头部之外的空间的信息为0
			
			    //write index header and buckets into index file
				ret=file_op->pwrite_file(init_data,indexHeader.index_file_offset,0);
				
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
			
			 // if(debug)printf("init block id:%d index successful. data file size:%d.index file offset:%d,bucket size:%d,free head offset:%d ,seq_no:%d,size:%d,file count:%d,del_size:%d,del_file_count:%d,version:%d\n",
			                   // logic_block_id, indexHeader.data_file_offset, indexHeader.index_file_offset, indexHeader.bucket_size, indexHeader.free_head_offset,
							   // indexHeader.block_info.seq_no, indexHeader.block_info.size, indexHeader.block_info.file_count, indexHeader.block_info.del_size,
							  // indexHeader.block_info.del_file_count, indexHeader.block_info.version);
							  
		     if(debug){
				 std::cout<<"the index header's pointer is:"<<index_header()<<std::endl;
				  printf("init block id:%d index successful. data file size:%d.index file offset:%d,bucket size:%d,free head offset:%d ,seq_no:%d,size:%d,file count:%d,del_size:%d,del_file_count:%d,version:%d\n",
			                   logic_block_id, index_header()->data_file_offset, index_header()->index_file_offset, 
							   index_header()->bucket_size,index_header()->free_head_offset,
							   blockInfo()->seq_no, blockInfo()->size, blockInfo()->file_count, blockInfo()->del_size,
							   blockInfo()->del_file_count, blockInfo()->version);
							  
			 }
			return linux_study::largefile::TFS_SUCCESS;
		}
		//加载索引块文件
		int IndexHandle::load(const uint32_t logic_block_id,const int32_t bucket_size,const MMapOption map_option){
			if(is_load){
			   fprintf(stderr,"in the loading of the index,the file has been loaded for the logic_block_id::%u\n",logic_block_id);
			   return linux_study::largefile::EXIT_INDEX_ALREADY_LOADED_ERROR;	
			}
		
			int ret=linux_study::largefile::TFS_SUCCESS;
			int64_t file_size=file_op->get_file_size();
			if(file_size<0){
				fprintf(stderr,"in the loading of the index,the file size is <0 for the logic_block_id::%u\n",logic_block_id);
				return file_size;
			}else if(file_size==0){//empty file that need init
			    fprintf(stderr,"in the loading of the index,the file size EMPTY for the logic_block_id::%u\n",logic_block_id);
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
				int32_t index_file_offset_=sizeof(IndexHeader)+bucket_size*sizeof(int32_t);
				
				if(file_size<index_file_offset_){
					fprintf(stderr,"Index corrupt error. block id :%u,bucket size:%d,file size:%ld,index file offset:%d\n",
					blockInfo()->block_id,bucketSize(),file_size,index_file_offset_);
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
				if(debug)printf("load block id:%d index successful. data file size:%d.index file offset:%d,bucket size:%d,free head offset:%d ,seq_no:%d,size:%d,file count:%d,del_size:%d,del_file_count:%d,version:%d\n",
			                 logic_block_id, index_header()->data_file_offset, index_header()->index_file_offset, 
							 index_header()->bucket_size,index_header()->free_head_offset,
							 blockInfo()->seq_no, blockInfo()->size, blockInfo()->file_count, blockInfo()->del_size,
							 blockInfo()->del_file_count, blockInfo()->version);
				return linux_study::largefile::TFS_SUCCESS;
				
			}
			
		}
		int IndexHandle::remove(const uint32_t logic_block_id){
			//判断是否已经加载
			if(!is_load){
				fprintf(stderr,"the index file is not loaded.the input block id:%d.\n",logic_block_id);
				return linux_study::largefile::EXIT_INDEX_NOT_LOADED_ERROR;
			}
			//判断传入的块id和当前持有的块id是否相同
			if(logic_block_id!=blockInfo()->block_id){
				fprintf(stderr,"block id conflict.the input block id:%d,index block id:%d.\n",logic_block_id,blockInfo()->block_id);
				return linux_study::largefile::EXIT_BLOCKID_CONFLICT_ERROR;
			}
			
			int ret=file_op->munmap_file();
			if(ret!=linux_study::largefile::TFS_SUCCESS){
				return ret;
			}
		    
			ret=file_op->unlink_file();
			return ret;
		}
		
		int IndexHandle::flush(){
			int ret=file_op->flush_file();
			if(linux_study::largefile::TFS_SUCCESS!=ret){
				fprintf(stderr,"index flush fail,ret:%d error desc:%s \n",ret,strerror(errno));
			}
			return ret;
		}
		
		int IndexHandle::write_segment_meta(const uint64_t key,MetaInfo & meta){
			int current_offset=0;
			int32_t previous_offset=0;
			
			
			//key是否存在
			int ret=hash_find(key,current_offset,previous_offset);
			
			//从文件哈希表中查找key是否存在 hash_find(key,current_offset,previous_offset)
			if(ret==linux_study::largefile::TFS_SUCCESS){
				return linux_study::largefile::EXIT_META_UNEXPECT_FOUND_ERROR;
			}
			else if(linux_study::largefile::EXIT_META_NOT_FOUND_ERROR!=ret){
				return ret;
			}
			//不存在就写入meta到文件哈希表中hash_insert(meta,slot);//TFS采取的是slot
			ret=hash_insert(key,previous_offset,meta);//采取key，offset，更直观一些
			return ret;
		}
		int32_t IndexHandle::hash_insert(const uint64_t key,int32_t previous_offset,MetaInfo &meta){
			int32_t slot=static_cast<uint32_t>(key%bucketSize());
			MetaInfo tmp_metaInfo;
			int ret=linux_study::largefile::TFS_SUCCESS;
			//找到存放当前节点的对应的偏移量
			int32_t current_offset =index_header()->index_file_offset;
			index_header()->index_file_offset+=sizeof(MetaInfo);
			
			//将MetaInfo节点写入索引文件中
			meta.set_next_meta_offset(0);
			
			ret=file_op->pwrite_file(reinterpret_cast<const char*>(&meta),sizeof(MetaInfo),current_offset);
			if(ret!=linux_study::largefile::TFS_SUCCESS){
				index_header()->index_file_offset-=sizeof(MetaInfo);//实行回滚，TFS中没有实现，但是既然写入失败，说明这个节点没有被使用
				return ret;
			}
			
			//把上一个节点的下一个节点指针指向当前节点,也就是正式插入哈希表中
			if(0!=previous_offset){
				ret=file_op->pread_file(reinterpret_cast<char*>(&tmp_metaInfo),sizeof(MetaInfo),previous_offset);
				
				if(ret!=linux_study::largefile::TFS_SUCCESS){
                 fprintf(stderr,"in the inserting meta_info,pread_file fail\n");
				return ret;
				}
				
				tmp_metaInfo.set_next_meta_offset(current_offset);
				ret=file_op->pwrite_file(reinterpret_cast<char*>(&tmp_metaInfo),sizeof(MetaInfo),previous_offset);
				
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					index_header()->index_file_offset-=sizeof(MetaInfo);//实行回滚，TFS中没有实现，但是既然写入失败，说明这个节点没有被使用
					return ret;
				}
			}
			else{
				bucket_slot()[slot]=current_offset;
			}
			return linux_study::largefile::TFS_SUCCESS;
			
			
		}
		int IndexHandle::hash_find(const uint64_t key,int32_t & current_offset,int32_t& previous_offset){
		    int ret=linux_study::largefile::TFS_SUCCESS;
			MetaInfo meta_info;
			
			current_offset=0;
			previous_offset=0;
			//查找key存放的桶(slot)的位置
			int32_t slot=(static_cast<int32_t>(key%bucketSize()));//slot达不到那么大，选择32，一般情况下不会超出，因为一个主块的大小是限定的
			//读取桶首节点存储的第一个节点的偏移量，如果偏移量为0，直接返回EXIT_META_NOT_FOUND_ERROR
			//根据偏移量读取存储的metaInfo
			//与key进行比较，相等则设置current_offset和previous_offset
			//否则继续执行,从metaInfo中读取next_meta_offset,，如果偏移量为0，直接返回EXIT_META_NOT_FOUND_ERROR，否则继续读取下一个mate
			int32_t pos=bucket_slot()[slot];
			for(  ;pos!=0;  ){
				//读取
				file_op->pread_file(reinterpret_cast<char*>(&meta_info),sizeof(MetaInfo),pos);
				//读取失败
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					return ret;
				}
				//比较hash值是否相等，相等则返回
				if(hash_compare(key,meta_info.get_key())){
					current_offset=pos;
					return linux_study::largefile::TFS_SUCCESS;
				}
				//第二个接待你开始才有前一个offset
				previous_offset=pos;
				//往下一个节点移动
				pos=meta_info.get_next_meta_offset();
				
			}
			return linux_study::largefile::EXIT_META_NOT_FOUND_ERROR;
		}
		
		void IndexHandle::commit_block_data_offset(const int file_size){
			reinterpret_cast<IndexHeader*>(file_op->get_map_data())->data_file_offset+=file_size;
		}
		
		int IndexHandle::update_block_info(const OperType oper_type,const uint32_t modify_size){
			if(blockInfo()->block_id==0){//block_id不能为零
				return linux_study::largefile::EXIT_BLOCKID_ZERO_ERROR;
			}
			if(oper_type==C_OPER_INSERT){
				++blockInfo()->version;
				++blockInfo()->file_count;
				++blockInfo()->seq_no;
				blockInfo()->size+=modify_size;
			}
			
			if(debug)printf("Update block info successfully.block_id:%d,version:%d, file count:%d, size:%d,del_file_count:%d,seq_no:%d,del_size:%d,oper_type:%d\n",
			                blockInfo()->block_id,blockInfo()->version, blockInfo()->file_count,
							 blockInfo()->size, blockInfo()->del_file_count,blockInfo()->seq_no,  blockInfo()->del_size,oper_type
							);
			return linux_study::largefile::TFS_SUCCESS;
			
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
		
		int32_t IndexHandle::get_block_data_offset()const{
			return reinterpret_cast<IndexHeader*>(file_op->get_map_data())->data_file_offset;
			
		}
		
		int32_t* IndexHandle::bucket_slot(){
			//跳过索引头信息IndexHeader然后跳转到MetaInfo哈希桶首节点地址
			return reinterpret_cast<int32_t*>(reinterpret_cast<char*>(file_op->get_map_data())+sizeof(IndexHeader));
		}
		
		bool IndexHandle::hash_compare(const uint64_t left_key,const uint64_t right_key)const{
			return left_key==right_key;
		}
					
	}
	
	
}