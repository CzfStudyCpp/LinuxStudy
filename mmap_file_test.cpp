//#define O_RDWR
//#define O_CREAT
//#define O_LARGEFILE
//上述针对大文件系统使用

#include"mmap_file.h"
#include"Public.h"

using namespace std;
using namespace linux_study;

static const mode_t OPEN_MODE=0644;//用户有读和写的权限，6=110
const static largefile::MMapOption mmap_option={10240000,4096,4096};//内存映射参数,代表内存大小

int open_file(string file_name,int open_flags){
	int fd=open(file_name.c_str(),open_flags,OPEN_MODE);//打开成功返回一个大于零的值
	if(fd<0){
		return -errno;//错误编号errno strerror(errno)获取错误原因
	}
	return fd;
}

int main(void){
	const char*filename="mapfile_test.txt";
	int fd = open_file(filename, O_RDWR | O_CREAT| O_LARGEFILE);
	if(fd<0){
		//这里采用fd，是因为errno是共享的，其他进程调用之后errno会被重置，使用fd就会保证不会被重置
		fprintf(stderr,"open file failed filename:%s.error desc:%s\n",filename,strerror(-fd));
		return -1;
	}
	largefile::MMapFile *map_file=new largefile::MMapFile(mmap_option,fd);
	
	bool is_mapped=map_file->map_file(true);
	if(is_mapped){
		map_file->remap_file();
		memset(map_file->get_data(),'9',map_file->get_size());
		map_file->sync_file();
		map_file->munmap_file();
	}
	else{
		fprintf(stderr,"map file failed");
	}
	close(fd);
	return 0;
	
	
}
