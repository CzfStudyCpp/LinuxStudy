#include<iostream>
#include<conio.h>

int process;
int resource; 

int maxs[10][10]; //最大需求矩阵
int allocation[10][10]; //分配矩阵
int need[10][10];   //需求矩阵
int available[10]; //可用资源向量
int request[10][10]; //请求矩阵
int compare_s(int Available[], int Need[]);
bool checkAvailable(int n, int* reque);
bool checkNeed(int n, int* reque);
void print();
//处理各个输入
void resourceInput()
{
    int i,j;

    std::cout<<"输入进程数量:\n";
	std::cin>>process;
	std::cout<<"输入资源种类数量:\n";
	std::cin>>resource;
    std::cout<<"请输入需求矩阵need\n";
	
    for(i=0; i<process; i++)
    {
        for(j=0; j<resource; j++)
        {
            std::cin>>need[i][j];
        }
    }
	std::cout<<"请输入分配矩阵allocation\n";
    for(i=0; i<process; i++)
    {
        for(j=0; j<resource; j++)
        {
           std::cin>>allocation[i][j];
        }
    }
    std::cout<<"请输入可用资源向量available\n";
    for(i=0; i<resource; i++)
    {
        std::cin>>available[i];
    }
}

//计算最大需求矩阵
void computeMax(){
	 for(int i=0; i<process; i++)
    {
        for(int j=0; j<resource; j++)
        {
           maxs[i][j]=need[i][j]+allocation[i][j];
        }
    }
	
	
}
//检查是否处于安全状态,找到安全序列
bool isSafe(){
	
	int finish[10];
	int work[10];
	for (int i = 0; i < process; i++)
	{
		finish[i] = 0;
	}
	for (int i = 0; i < resource; i++)
	{
		work[i] = available[i];
	}
	std::cout << "分配序列：\n";
	std::cout << "        \tallocation         \tneed          \t进程完成后的avilable" << std::endl;
	for (int k = 0; k < process; k++)
	{
		for (int i = 0; i < process; i++)
		{
			if (finish[i] == 1)
			{
				continue;
			}
			else
			{
				if (compare_s(work, need[i]))//available>=need
				{
					finish[i] = 1;
					std::cout << '\n' << "进程" << i << '\t';
					for (int j = 0; j < resource; j++)
					{
						printf(" %2d ", allocation[i][j]);
					}
					std::cout << "     ";
					for (int j = 0; j < resource; j++)
					{
						printf(" %2d ", need[i][j]);
					}
					std::cout << "     ";
					for (int j = 0; j < resource; j++)
					{
						work[j] += allocation[i][j];
						printf(" %2d ", work[j]);
					}
				
				}
			}
		}
	}
	std::cout << '\n';
	for (int l = 0; l < process; l++)
	{
		if (finish[l] == 0)
		{
			return false;//不存在安全序列
		}
	}
	return true;//存在安全序列

}
//分配资源
int Allocate(int n,int *reque){
	
	if (checkNeed(n, reque) && checkAvailable(n, reque)) {
		for (int j = 0; j < resource; j++) {
			allocation[n][j] += reque[j];
			available[j] -= reque[j];
			need[n][j] -= reque[j];
		}
		return 1;
	}
	else if (!checkNeed(n, reque)) {
		return 2;
	}
	else if (!checkAvailable(n, reque))
		return 3;
	return 0;
	
}
//检查请求资源是否超过需要
bool checkNeed(int n,int *reque){
	for(int i=0;i<resource;i++){
		if(reque[i]>need[n][i]) {
			return false;
		   
		}
	}
	return true;
	
}
//检查请求资源是否超过空闲资源
bool checkAvailable(int n,int *reque){
	for(int i=0;i<resource;i++){
		if(reque[i]>available[i]){
		    return false;
		}
	}
	return true;
	
}

int compare_s(int Available[],int Need[])
{
    int i;
    for(i=0; i<resource; i++)
    {
        if(Available[i]<Need[i])
        {
            return 0;
        }
    }
    return 1;
}
void test1(){
	std::cout << "连续请求独立测试，互不影响（按q退出）:\n";
	resourceInput();
	computeMax();
	if (isSafe()) {
		std::cout << "t0时刻安全\n";
	}
	else std::cout << "t0时刻不安全\n";
	int sub_need[10][10];
	int sub_allocation[10][10];
	int sub_available[10];
	for(int i=0;i<process;i++){
		for(int j=0;j<resource;j++){
			sub_need[i][j]=need[i][j];
			 sub_allocation[i][j]=allocation[i][j];
	        
		}
	}
	for (int j = 0; j < resource; j++)
		sub_available[j] = available[j];
	while(1){
		
		if (_kbhit()) // 如果有按键被按下
		{
			if (_getch() == 'q') //如果按下了q键则跳出循环
			{
				break;
			}

		}
		
		
		std::cout<<"输入需要请求资源的进程编号:\n";
		int number;
		std::cin>>number;
		std::cout<<"输入请求资源向量:\n";
		//int *ask_for=new int[resource];
		for(int i=0;i<resource;i++)
		{
			std::cin>>request[number][i];
		}
		std::cout<<"处理请求...\n";
		int temp = Allocate(number, request[number]);

		if (temp==1)
		{
			if (isSafe())
			
				std::cout << "进程" << number << "请求资源是安全的\n";
			else
				std::cout << "进程" << number << "请求资源是不安全的\n";

			print();
		}
		else if(temp==2)
			std::cout << "进程" << number << "请求资源超出最大需求\n";
		else if (temp == 3)
			std::cout << "进程" << number << "请求资源超出系统空闲资源\n";
		
		
		for(int i=0;i<process;i++){
		  for(int j=0;j<resource;j++){
			need[i][j]=sub_need[i][j];
			 allocation[i][j]=sub_allocation[i][j];
	         
		  }
	    }
		for (int j = 0; j < resource; j++) 
			available[j] = sub_available[j];
	}
	
}

void test2(){
	
	std::cout << "连续非独立测试，受前面进程请求影响,按q退出:\n";
	resourceInput();
	computeMax();
	if (isSafe()) {
		std::cout << "t0时刻安全\n";
	}
	else std::cout << "t0时刻不安全\n";

	std::cout << "输入需要请求资源的进程编号:\n";
	while(1){
		
		if (_kbhit()) // 如果有按键被按下
		{
			if (_getch() == 'q') //如果按下了q键则跳出循环
			{
				break;
			}

		}
		std::cout << "输入请求进程编号:\n";
		int number;
		std::cin>>number;
		std::cout<<"输入请求资源向量:\n";
		//int *ask_for=new int[resource];
		for(int i=0;i<resource;i++)
		{
			std::cin>>request[number][i];
		}
		std::cout<<"处理请求...\n";
		int temp=Allocate(number,request[number]);
		if (temp == 1)
		{
			if (isSafe())
				std::cout << "进程" << number << "请求资源是安全的\n";
			else
				std::cout << "进程" << number << "请求资源是不安全的\n";
			print();
		}
		else if (temp == 2)
			std::cout << "进程" << number << "请求资源超出最大需求\n";
		else if (temp == 3)
			std::cout << "进程" << number << "请求资源超出系统空闲资源\n";
	}
	
}

void print() {
	std::cout << " 当前分配状况\n";
	std::cout << "  allocation     \tneed " << std::endl;
	for (int i = 0; i < process; i++) {
		std::cout << "   ";
		for (int j = 0; j < resource; j++) {
			std::cout << allocation[i][j] << "  ";
		}
		std::cout << "   ";
		for (int j = 0; j < resource; j++) {
			std::cout <<need[i][j] << "  ";
		}
		std::cout << "   ";
		std::cout << "\n";
	}

	std::cout << "avilable" << std::endl;
	for (int j = 0; j < resource; j++) {
		std::cout << available[j] << "  ";
	}
	std::cout<< std::endl;
}
int main(){
    //test1();
	test2();
  
}
