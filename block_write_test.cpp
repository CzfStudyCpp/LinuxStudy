//在主块目录中创建主块文件
//在索引目录中创建索引文件

//文件写入主块
//索引信息写入索引文件

#include"Public.h"
#include"file_op.h"
#include"index_handle.h"
#include<sstream>


using namespace linux_study;

const static largefile::MMapOption mmap_option ={1024000,4096,4096};//映射内存文件大小

const static uint32_t mainBlockSize=1024*1024*64;//主块文件大小

const static uint32_t bucket_size=1000;//哈希桶的大小

static int32_t block_id=1;

const static int debug=1;

int main(int argc,char **argv){//argc 表示参数数量，argv存放参数 rm -f a.txt  表示三个参数 

    std::string mainBlock_path;//主块路径
	std::string index_path;//索引路径
	
    std::cout<<"type your block id:\n";
	std::cin>>block_id;
	
	if(block_id<1){
		std::cerr<<"invalid block id ,exit\n";
		exit(-1);
	}
	int ret=largefile::TFS_SUCCESS;//反馈信息
	
	//load index file
	
	largefile::IndexHandle*index_handle =new largefile::IndexHandle(".",block_id);
	
	if(debug)printf("load index...\n");
	
	ret=index_handle->load(block_id,bucket_size,mmap_option);//加载信息
	
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"load index with block id :%d failed，reason:%s.\n",block_id,strerror(errno));
		
		delete index_handle;
		exit(-2);
	}
	
	//writhe files into the mainBlock file
	std::stringstream tmp_stream;
	tmp_stream<<"."<<largefile::MAINBLOCK_DIR_PREFIX<<block_id;
	tmp_stream>>mainBlock_path;
	
	largefile::FileOperation * mainBlock=new largefile::FileOperation(mainBlock_path,O_RDWR|O_LARGEFILE|O_CREAT);
	
	char buffer[4096];
	memset(buffer,'9',sizeof(buffer));
	int32_t data_offset=index_handle->get_block_data_offset();
	printf("The data_offset by the index_handle get is :%d\n",data_offset);
	uint32_t file_no=index_handle->blockInfo()->seq_no;
	printf("The file_no by the index_handle get is :%d\n",file_no);
	if((ret=mainBlock->pwrite_file(buffer,sizeof(buffer),data_offset))!=linux_study::largefile::TFS_SUCCESS){
		fprintf(stderr,"write to main block failed.ret:%d.reason:%s\n",ret,strerror(errno));
		mainBlock->close_file();
		delete mainBlock;
		delete index_handle;
		exit(-3);
		
	}
	
	//3.索引文件中写入MetaInfo
	largefile::MetaInfo meta;
	meta.set_file_id(file_no);
	meta.set_offset(data_offset);
	meta.set_size(sizeof(buffer));
	
	ret=index_handle->write_segment_meta(meta.get_key(),meta);
	if(ret==largefile::TFS_SUCCESS){
		//更新四索引头部信息,,更新版本号，下一个可分配的文件编号，未使用数据起始的偏移量
		index_handle->commit_block_data_offset(sizeof(buffer));//更新数据偏移信息
		//1.更新块信息,,
		ret=index_handle->update_block_info(largefile::C_OPER_INSERT,sizeof(buffer));//更新块信息
		if(ret!=largefile::TFS_SUCCESS){
			fprintf(stderr,"update_block_info failed.ret:%d.reason:%s\n",ret,strerror(errno));
		}
		ret =index_handle->flush();//刷新持久化
		
		if(ret!=largefile::TFS_SUCCESS){
			fprintf(stderr,"flush the index file with the block id %d failed,file no : %u\n",block_id,file_no);
		}
		
		
		//
	}
	else{
		fprintf(stderr,"write_segement_meta -mainblock %d failed.file no %u \n",block_id,file_no);
		
	}
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"write to mainBlock %d failed.file no %u \n",block_id,file_no);
	}
	else{
		if(debug)printf("write successfully.file_no:%u,block_id:%d\n",file_no,block_id);
	}
	
	
	// if(ret!=0){
		// fprintf(stderr,"create main block %s failed.reason:%s\n",
		// mainBlock_path.c_str(),strerror(errno));
		// delete mainBlock;
		// //删除索引文件
		// index_handle->remove(block_id);
		// exit(-2);
	// }
	//关闭主块文件
	mainBlock->close_file();
	//index_handle->flush();
	
	delete mainBlock;
	delete index_handle;
      
	mainBlock=nullptr;
	index_handle=nullptr;
	return 0;
}
