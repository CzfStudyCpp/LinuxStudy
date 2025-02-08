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
	
	
	//读取文件的metaInfo
	uint64_t file_id=0;
	std::cout<<"input your fild_id:"<<std::endl;
    std::cin>>file_id;
	
	if(file_id<1){
		fprintf(stderr,"the file id is <1\n");
		exit(-1);
	}
	
	largefile::MetaInfo meta;
	
	ret= index_handle->read_segment_meta(file_id,meta);
	if(ret!=largefile::TFS_SUCCESS){
	   fprintf(stderr,"Read the index file fail with the file_id:%lu,ret:%d",file_id,ret);
	   exit(-3);
	}
	
	//根据metaInfo读取文件
	std::stringstream tmp_stream;
	tmp_stream<<"."<<largefile::MAINBLOCK_DIR_PREFIX<<block_id;
	tmp_stream>>mainBlock_path;
	
	largefile::FileOperation * mainBlock=new largefile::FileOperation(mainBlock_path,O_RDWR);
	
	char buffer[meta.get_size()+1];//测试环境下，文件不大，可以这么写着方便
	ret = mainBlock->pread_file(buffer,meta.get_size(),meta.get_offset());
	if(ret!=linux_study::largefile::TFS_SUCCESS){
		fprintf(stderr,"read  main block failed.ret:%d.reason:%s\n",ret,strerror(errno));
		mainBlock->close_file();
		delete mainBlock;
		delete index_handle;
		exit(-3);
		
	}
	buffer[meta.get_size()+1]='\0';
	printf("the read size is : %d,the data read is : %s\n",meta.get_size(),buffer);
	
	// if(ret!=0){
		// fprintf(stderr,"create main block %s failed.reason:%s\n",
		// mainBlock_path.c_str(),strerror(errno));
		// delete mainBlock;
		// //删除索引文件
		// index_handle->remove(block_id);
		// exit(-2);
	// }
	//关闭主块文件
	//index_handle->flush();
	delete mainBlock;
	delete index_handle;
      
	index_handle=nullptr;
	return 0;
}
