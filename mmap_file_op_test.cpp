#include<iostream>
#include "mmap_file_op.h"

using namespace std;

using namespace linux_study;

const static largefile::MMapOption mmap_option={10240000,4096,4096};//内存映射参数,代表内存大小
int main(void){
	int ret=0;
	
	const char *filename="mmap_file_op.txt";
	largefile::MMapFileOperation *mmfo;
	mmfo=new largefile::MMapFileOperation(filename);
	int fd=mmfo->open_file();
	if(fd<0){
		fprintf(stderr,"open file %s failed reason:%s\n",filename,strerror(-fd));
		exit(-1);
	}
	ret=mmfo->mmap_file(mmap_option);
	if(ret==linux_study::largefile::TFS_ERROR){
		fprintf(stderr,"mmap_file failed.reason:%s\n" ,strerror(errno));
		mmfo->close_file();
		//exit(-2);
	}
	char buffer[129];
	memset(buffer,'6',128);
	
	 ret =mmfo->pwrite_file(buffer,128,8192);
	if(ret<0){
		if(ret==largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr,"pwrite length is less than required!\n");
		}
		else{
		fprintf(stderr,"pwrite file %s failed reason:%s\n",filename,strerror(-fd));
		//exit(-2);
		}
	}
	
	memset(buffer,0,128);
	ret=mmfo->pread_file(buffer,10,4096);
	if(ret<0){
		if(ret==largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr,"pread length is less than required!\n");
		}
		else{
		fprintf(stderr,"pread file %s failed reason:%s\n",filename,strerror(-fd));
		//exit(-3);
		}
	}
	else{
		buffer[129]='\0';
		printf("read:%s\n",buffer);
	}
	ret=mmfo->flush_file();
	if(ret==largefile::TFS_ERROR){
		fprintf(stderr,"flush file failed.reason:%s\n",strerror(errno));
	}
	mmfo->munmap_file();
	mmfo->close_file();
	delete(mmfo);
	return 0;	
	
}