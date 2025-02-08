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
	
	
	//删除文件的metaInfo
	uint64_t file_id=0;
	std::cout<<"input your fild_id:"<<std::endl;
    std::cin>>file_id;
	
	if(file_id<1){
		fprintf(stderr,"the file id is < 1\n");
		exit(-1);
	}
	
	ret=index_handle->remove_segment_meta(file_id);
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"remove the index failed with the fild_id:%lu,ret:%d",file_id,ret);
		exit(-3);
	}
	
	ret =index_handle->flush();//刷新持久化
		
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"flush the index file with the block id %d failed,file id : %lu\n",block_id,file_id);
		exit(-3);
	}
	
	//索引文件的删除，将删除的meta块加入可用块空间
	//而主块的空间则只是加个标记,当达到某个阈值的时候进行集中的磁盘空间整理
	
	//因为写入操作需要确保主块和索引都正确写入，所以需要全部完成才更新块信息
	//而删除操作则是默认文件删除了，那么可以在删除函数里面加入更新操作

	printf("delete successfully!\n");

	delete index_handle;
      
	index_handle=nullptr;
	return 0;
}
