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
				char* init_data =new char[indexHeader.index_file_offset+1];//数据信息缓冲
				memcpy(init_data,&indexHeader,sizeof(indexHeader));//将头部信息复制
				memset(init_data+sizeof(IndexHeader),0,indexHeader.index_file_offset-sizeof(indexHeader));//初始化设置头部之外的空间的信息为0
			    init_data[indexHeader.index_file_offset]='\0';
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
				if(debug)printf("load block id:%d index successful. data file offset:%d.index file offset:%d,bucket size:%d,free head offset:%d ,seq_no:%d,size:%d,file count:%d,del_size:%d,del_file_count:%d,version:%d\n",
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
			int32_t current_offset=0;
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
		
		int IndexHandle::read_segment_meta(const uint64_t key,MetaInfo & meta){
			int32_t current_offset=0;
			int32_t previous_offset=0;
			
			//key是否存在
			int ret=hash_find(key,current_offset,previous_offset);
			if(ret==linux_study::largefile::TFS_SUCCESS){
				ret=file_op->pread_file(reinterpret_cast<char*>(&meta),sizeof(MetaInfo),current_offset);
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"in the read_segment_meta,the pread_filef failed with the ret:%d,key :%lu",ret,key);
					return ret;
				}
				return ret;
			}else{
				fprintf(stderr,"in the read_segment_meta,the key is not found or the hash_find fail with the ret:%d,key :%lu",ret,key);
				return ret;
			}
			
			
		}
		
		int IndexHandle::remove_segment_meta(const uint64_t key){
			int32_t current_offset=0;
			int32_t previous_offset=0;
			
			//key是否存在
			int ret=hash_find(key,current_offset,previous_offset);
			if(ret!=linux_study::largefile::TFS_SUCCESS){
				fprintf(stderr,"in removing index meta function,the key is not found with the key:%lu\n",key);
				return ret;
			}
			
			MetaInfo meta;
			
			ret=file_op->pread_file(reinterpret_cast<char*>(&meta),sizeof(MetaInfo),current_offset);
			if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"in the remove_segment_meta,the pread_file of the current_offset :%d failed with the ret:%d,key :%lu",current_offset,ret,key);
					return ret;
			}
			int32_t next_pos=meta.get_next_meta_offset();
			
			if(previous_offset==0){
				//找到桶的位置
				int32_t slot=static_cast<uint32_t>(key)%bucketSize();
				
				bucket_slot()[slot]=next_pos;//修正下一个节点的位置
			}else{
				
				MetaInfo previous_meta;
				ret=file_op->pread_file(reinterpret_cast<char*>(&previous_meta),sizeof(MetaInfo),previous_offset);//读取上一个节点
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"in the remove_segment_meta,the pread_file of the previous_offset :%d failed with the ret:%d,key :%lu",previous_offset,ret,key);
					return ret;
				}
				//修改要删除的节点的上一个节点指向当前节点的下一个节点
				previous_meta.set_next_meta_offset(next_pos);
				
				ret=file_op->pwrite_file(reinterpret_cast<char*>(&previous_meta),sizeof(MetaInfo),previous_offset);//持久化存储
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"in the remove_segment_meta,the pwrite_file of the previous_offset :%d failed with the ret:%d,key :%lu",previous_offset,ret,key);
					return ret;
				}
			}
			//前插法加入删除节点
			//把删除的节点加入自由可用节点链表
			meta.set_next_meta_offset(get_free_head_offset());
			
			ret=file_op->pwrite_file(reinterpret_cast<char*>(&meta),sizeof(MetaInfo),current_offset);//持久化存储
			if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"in the remove_segment_meta,the pwrite_file of the current_offset :%d failed with the ret:%d,key :%lu",current_offset,ret,key);
					return ret;
			}
			index_header()->free_head_offset=current_offset;
			
			
			//更新块信息
			update_block_info(C_OPER_DELETE,meta.get_size());
			
			if(debug)printf("delete_meta successfully and the next_pos is:%d,the new free_head_offset also the current_offset is:%d\n",next_pos,index_header()->free_head_offset);
			return linux_study::largefile::TFS_SUCCESS;
			
		}
		int32_t IndexHandle::hash_insert(const uint64_t key,int32_t previous_offset,MetaInfo &meta){
			int32_t slot=static_cast<uint32_t>(key)%bucketSize();
			MetaInfo tmp_metaInfo;
			int ret=linux_study::largefile::TFS_SUCCESS;
			
			int32_t current_offset=0;
			//找到被删除的可重用的节点
			int32_t free_head_offset_=get_free_head_offset();
			if(free_head_offset_!=0){
				ret=file_op->pread_file(reinterpret_cast<char*>(&tmp_metaInfo),sizeof(MetaInfo),free_head_offset_);
				
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"in the hash_insert,the pread_file of the free_head_offset_ :%d failed with the ret:%d,key :%lu",free_head_offset_,ret,key);
					return ret;
				}
				 current_offset=get_free_head_offset();
				 index_header()->free_head_offset=tmp_metaInfo.get_next_meta_offset();
				 
				 if(debug)printf("reuse free meta,the reuse current_offset is:%d,the new free_head_offset is:%d\n",current_offset,index_header()->free_head_offset);
			}else{
				//找到存放当前节点的对应的偏移量
			    current_offset =index_header()->index_file_offset;
			    index_header()->index_file_offset+=sizeof(MetaInfo);
			
			}
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
				////表示第一个节点
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
			int32_t slot=(static_cast<int32_t>(key)%bucketSize());//slot达不到那么大，选择32，一般情况下不会超出，因为一个主块的大小是限定的
			//读取桶首节点存储的第一个节点的偏移量，如果偏移量为0，直接返回EXIT_META_NOT_FOUND_ERROR
			//根据偏移量读取存储的metaInfo
			//与key进行比较，相等则设置current_offset和previous_offset
			//否则继续执行,从metaInfo中读取next_meta_offset,，如果偏移量为0，直接返回EXIT_META_NOT_FOUND_ERROR，否则继续读取下一个mate
			int32_t pos=bucket_slot()[slot];
			printf("in the hash_find start!,the first slot found by the key:%lu is :%d,with the pos is :%d\n\n",key,slot,pos);
			for(  ;pos!=0;  ){
				//读取
				file_op->pread_file(reinterpret_cast<char*>(&meta_info),sizeof(MetaInfo),pos);
				//读取失败
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"in the hash_find,read the index meta_info fail\n");
					return ret;
				}
				if(debug){
					printf("the read meta_info :the file_id is:%lu,the next_meta_offset is :%d,and the file in the main block the info is :the inner_offset in the block is :%d,the file size is:%d",
					        meta_info.get_file_id(),meta_info.get_next_meta_offset(),meta_info.get_offset(),meta_info.get_size());
				}
				//比较hash值是否相等，相等则返回
				if(hash_compare(key,meta_info.get_key())){
					current_offset=pos;
					printf("in the hash_find for if judge find the key successfully!,the first slot found by the key:%lu is :%d,and the needed pos is :%d\n\n",key,slot,pos);
					return linux_study::largefile::TFS_SUCCESS;
				}
				//第二个节点开始才有前一个offset
				previous_offset=pos;
				//往下一个节点移动
				pos=meta_info.get_next_meta_offset();
				printf("in the hash_find for continue to find next pos!,the first slot found by the key:%lu is :%d,the previous_offset :%d, the next pos is :%d\n\n",key,slot,previous_offset,pos);
				
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
			}else if(oper_type==C_OPER_DELETE){
				++blockInfo()->version;
				--blockInfo()->file_count;
				
				blockInfo()->size-=modify_size;
				
				++blockInfo()->del_file_count;
				blockInfo()->del_size+=modify_size;
				
			}
			
			if(debug)printf("Update block info successfully.block_id:%d,version:%d, file count:%d, size:%d,del_file_count:%d,seq_no:%d,del_size:%d,oper_type:%d\n",
			                blockInfo()->block_id,blockInfo()->version, blockInfo()->file_count,
							 blockInfo()->size, blockInfo()->del_file_count,blockInfo()->seq_no,  blockInfo()->del_size,oper_type
							);
			return linux_study::largefile::TFS_SUCCESS;
			
		}
		int32_t IndexHandle::get_meta_list(std::vector<linux_study::largefile::MetaInfo>& meta_list) {
			int ret = linux_study::largefile::TFS_SUCCESS;

			int32_t previous_offset = 0;
			int32_t bucket_size_ = bucketSize();
			for (int i = 0; i < bucket_size_; i++) {
				// 该槽位为空
				if (bucket_slot()[i] == 0) continue;

				int32_t pos = bucket_slot()[i];
				while (pos != 0) {
					MetaInfo meta; // 修正：在这里添加分号
					ret = file_op->pread_file(reinterpret_cast<char*>(&meta), sizeof(MetaInfo), pos); // 需要赋值 ret

					// 读取失败
					if (ret != linux_study::largefile::TFS_SUCCESS) {
						fprintf(stderr, "in the get_meta_list, read the index meta_info fail\n");
						return ret;
					}
					meta_list.push_back(meta);

					// 第二个节点开始才有前一个offset
					previous_offset = pos;
					// 往下一个节点移动
					pos = meta.get_next_meta_offset(); // 确保此处的 meta 是最新的
					if (debug) {
						printf("in the get_meta_list for continue to find next pos!, the slot is: %d, the previous_offset: %d, the next pos is: %d\n\n", i, previous_offset, pos);
					}
				}
			}
			return linux_study::largefile::TFS_SUCCESS;
		}

		
		int32_t IndexHandle::reorder_index(std::vector<MetaInfo>&usefulMetaList){
			size_t meta_numbers=usefulMetaList.size();
			reset_the_bucket_slot();
			//这一步重新设置每个桶的数据信息，以及对应的哈希链表
			int32_t ret=linux_study::largefile::TFS_SUCCESS;
			for(auto & meta:usefulMetaList){
				
				ret=write_segment_meta(meta.get_file_id(),meta);
				if(ret!=linux_study::largefile::TFS_SUCCESS){
					fprintf(stderr,"in reorder_index,write_segment_meta fail with the file_id:%lu,and the RET is :%d\n",meta.get_file_id(),ret);
					return ret;
				}
				//槽位的首节点设置为
			}
			int32_t file_count_=usefulMetaList.size();
			index_header()->data_file_offset=(usefulMetaList[file_count_-1].get_offset()+usefulMetaList[file_count_-1].get_size());
			blockInfo()->size=index_header()->data_file_offset;
			blockInfo()->file_count=file_count_;
			blockInfo()->seq_no=file_count_+1;
			blockInfo()->version++;
			file_op->flush_file();
			if(debug)printf("reorder_index successfully!\n the new data_file_offset is:%d\t,the new index_file_offset is :%d,the free_head_offset is:%d\t"
			                "the new file_count is :%d\t,new size is:%d\t,new seq_no is:%d\n"
			         ,index_header()->data_file_offset,index_header()->index_file_offset,index_header()->free_head_offset,
					 blockInfo()->file_count,blockInfo()->size,blockInfo()->seq_no);
			return linux_study::largefile::TFS_SUCCESS;
			
		}
	    void IndexHandle::reset_the_bucket_slot(){
			//将桶的索引信息全部归0
			int32_t index_bucket_size=(bucketSize()*sizeof(int32_t));
			char *bucket_data=new char[index_bucket_size+1];
			memset(bucket_data,0,index_bucket_size);
			bucket_data[index_bucket_size]='\0';
			file_op->pwrite_file(bucket_data,index_bucket_size,sizeof(IndexHeader));
			//索引头可以重置偏移量，数据块偏移量可以后面再修改
			index_header()->index_file_offset=sizeof(IndexHeader)+bucketSize()*sizeof(int32_t);
			index_header()->free_head_offset=0;
			//块信息除了版本号和块id其他都需要进行修改，已删除的文件信息可以重置，其他变量可以在重排索引的时候进行修改
			blockInfo()->del_file_count=0;
			blockInfo()->del_size=0;
			delete bucket_data;
			bucket_data=nullptr;
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
		
		int32_t IndexHandle::get_free_head_offset()const{
			return reinterpret_cast<IndexHeader*>(file_op->get_map_data())->free_head_offset;
			
		}
		bool IndexHandle::hash_compare(const uint64_t left_key,const uint64_t right_key)const{
			return left_key==right_key;
		}
					
	}
	
	
}