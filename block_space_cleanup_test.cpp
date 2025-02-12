//清理主块空间
//方案一：采取双缓冲进行整理，创建一个新的块，将旧块的数据重新排列到新块中，删除旧块
//方案二：批量整理，读取所有有效文件，计算它们整理后的新位置

#include"Public.h"
#include"file_op.h"
#include"index_handle.h"
#include<sstream>
#include <algorithm> 
using namespace linux_study;

const static largefile::MMapOption mmap_option ={1024000,4096,4096};//映射内存文件大小

const static uint32_t mainBlockSize=1024*1024*64;//主块文件大小

const static uint32_t bucket_size=2;//哈希桶的大小

static int32_t block_id=1;

const static int debug=1;

//metaInfo 的数据块偏移量比较器
bool compareMetaInfo(const largefile::MetaInfo &a, const largefile::MetaInfo &b) {
    return a.get_offset() < b.get_offset();
}

//双缓冲进行整理
int doubleBufferCleanUp(largefile::IndexHandle*&indexHandle,largefile::FileOperation *&mainBlock,int32_t blockId){
    //读取索引文件中的所有有效文件元数据,组成有效文件元数据数组
	//创建新的主块
	//根据有效文件元数据复制旧数据块的数据到新的数据块
	//删除旧的数据块
	//将新的数据块更名为旧数据块的名
	//清空索引文件的可重用文件元数据链表
	printf("正在进行双缓冲空间整理模式>>>the blockId :%d\n",blockId);
	std::vector<largefile::MetaInfo>usefulMetaList;
	int ret=indexHandle->get_meta_list(usefulMetaList);
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"in doubleBufferCleanUp get_meta_list fail with the RET is :%d\n",ret);
		return ret;
	}
	if(usefulMetaList.size()==0){
		fprintf(stderr,"in doubleBufferCleanUp usefulMetaList is empty with the ret is :%d\n",ret);
		return ret;
	}
	std::stringstream tmp_stream;
	
	tmp_stream<<"."<<largefile::MAINBLOCK_DIR_PREFIX<<largefile::BLOCK_BACKUP<<block_id;
	
	std::string newMainBlockPath=tmp_stream.str();
	
	largefile::FileOperation* newMainBlock=new largefile::FileOperation(newMainBlockPath,O_RDWR|O_LARGEFILE|O_CREAT);
	ret=newMainBlock->ftruncate_file(mainBlockSize);
	
	if(ret!=0){
		fprintf(stderr,"create main block %s failed.reason:%s\n",
		newMainBlockPath.c_str(),strerror(errno));
		newMainBlock->unlink_file();
		delete newMainBlock;
		exit(-2);
	}
	ret=newMainBlock->copy_main_block(mainBlock,usefulMetaList);
	
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"in doubleBufferCleanUp copy_main_block fail with the RET is :%d\n",ret);
		return ret;
	}
	//newMainBlock->flush_file();
	// 重置流内容
    tmp_stream.str("");
    tmp_stream.clear();
	
	tmp_stream<<"."<<largefile::MAINBLOCK_DIR_PREFIX<<block_id;
	std::string resetOldBlockName=tmp_stream.str();
	//删除当前旧块新建主块
	ret=mainBlock->unlink_file();
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"in doubleBufferCleanUp unlink_file fail with the RET is :%d\n",ret);
		return ret;
	}
	printf("旧主块已完成删除操作,将删除旧主块指针\n");
	delete mainBlock;
	mainBlock=nullptr;
	
	 printf("将新块更名为旧块信息\n");
	ret=newMainBlock->rename_file(resetOldBlockName,newMainBlockPath);
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"in doubleBufferCleanUp rename fail with the RET is :%d\n",ret);
		return ret;
	}
	delete newMainBlock;
	newMainBlock=nullptr;
	//索引整理，重新整理索引
	 printf("将重新排列索引\n");
	ret=indexHandle->reorder_index(usefulMetaList);
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"in doubleBufferCleanUp reorder_index fail with the RET is :%d\n",ret);
		return ret;
	}
	return largefile::TFS_SUCCESS;
}

int32_t batchCleanUp(largefile::IndexHandle*&indexHandle,largefile::FileOperation *&mainBlock,int32_t blockId){
	//读取索引文件中的所有有效文件元数据,组成有效文件元数据数组
	//对元数据数组按照数据块偏移量进行从小到大的排序
	//在需要整理的主块对象里进行批量整理，将所有文件按照偏移量先后顺序进行向前移动
	//重新整理索引文件索引
	printf("正在进行批量空间整理模式>>>the blockId :%d\n",blockId);
	std::vector<largefile::MetaInfo>usefulMetaList;
	int ret=indexHandle->get_meta_list(usefulMetaList);
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"in batchCleanUp get_meta_list fail with the RET is :%d\n",ret);
		return ret;
	}
	if(usefulMetaList.size()==0){
		fprintf(stderr,"in batchCleanUp usefulMetaList is empty with the ret is :%d\n",ret);
		return ret;
	}
	std::sort(usefulMetaList.begin(), usefulMetaList.end(), compareMetaInfo);
	mainBlock->batch_clean_up(usefulMetaList);
	printf("已完成批量空间整理,将重新排列索引\n");
	ret=indexHandle->reorder_index(usefulMetaList);
	if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"in doubleBufferCleanUp reorder_index fail with the RET is :%d\n",ret);
		return ret;
	}
	printf("已完成批量空间整理\n");
	return largefile::TFS_SUCCESS;
	
}
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
	
	std::stringstream tmp_stream;
	tmp_stream<<"."<<largefile::MAINBLOCK_DIR_PREFIX<<block_id;
	mainBlock_path=tmp_stream.str();
	
	largefile::FileOperation * mainBlock=new largefile::FileOperation(mainBlock_path,O_RDWR);
	
	printf("Start the block clean up with the block id :%d\n",block_id);
	//ret=doubleBufferCleanUp(index_handle,mainBlock,block_id);
	ret=batchCleanUp(index_handle,mainBlock,block_id);
    if(ret!=largefile::TFS_SUCCESS){
		fprintf(stderr,"clean up the block :%d fail with the ret is :%d",block_id,ret);	
		delete index_handle;
		delete mainBlock;
		exit(-2);
	}
	printf("clean the block : %d successfully\n",block_id);
	return 0;
}
