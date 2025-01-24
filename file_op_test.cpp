#include"file_op.h"
#include"Public.h"

using namespace std;
using namespace linux_study;
int main(){
	const char*filename="file_op.txt";
	largefile::FileOperation *fileOP=new largefile::FileOperation(filename,O_CREAT|O_RDWR|O_LARGEFILE);
	int fd = fileOP->open_file();
	if(fd<0){
		fprintf(stderr,"open file %s failed reason:%s\n",filename,strerror(-fd));
		exit(-1);
	}
	char buffer[65];
	
	memset(buffer,'6',64);
	
	int ret=fileOP->pwrite_file(buffer,64,1024);
	if(ret<0){
		if(ret==largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr,"pwrite length is less than required!\n");
		}
		else{
		fprintf(stderr,"pwrite file %s failed reason:%s\n",filename,strerror(-fd));
		exit(-2);
		}
	}
	
	memset(buffer,0,64);
	ret=fileOP->pread_file(buffer,64,1024);
	if(ret<0){
		if(ret==largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr,"pread length is less than required!\n");
		}
		else{
		fprintf(stderr,"pread file %s failed reason:%s\n",filename,strerror(-fd));
		exit(-3);
		}
	}
	else{
		buffer[65]='\0';
		printf("read:%s\n",buffer);
	}
	
	memset(buffer,'9',64);
	ret=fileOP->write_file(buffer,64);
	if(ret<0){
		if(ret==largefile::EXIT_DISK_OPER_INCOMPLETE){
			fprintf(stderr,"write length is less than required!\n");
		}
		else{
			fprintf(stderr,"write file %s failed reason:%s\n",filename,strerror(-fd));
			exit(-3);
		}
	}
	
	fileOP->close_file();
	
	delete(fileOP);
	
	return 0;
}
	