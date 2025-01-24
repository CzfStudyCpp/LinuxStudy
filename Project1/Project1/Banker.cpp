#include<iostream>
#include<conio.h>

int process;
int resource; 

int maxs[10][10]; //����������
int allocation[10][10]; //�������
int need[10][10];   //�������
int available[10]; //������Դ����
int request[10][10]; //�������
int compare_s(int Available[], int Need[]);
bool checkAvailable(int n, int* reque);
bool checkNeed(int n, int* reque);
void print();
//�����������
void resourceInput()
{
    int i,j;

    std::cout<<"�����������:\n";
	std::cin>>process;
	std::cout<<"������Դ��������:\n";
	std::cin>>resource;
    std::cout<<"�������������need\n";
	
    for(i=0; i<process; i++)
    {
        for(j=0; j<resource; j++)
        {
            std::cin>>need[i][j];
        }
    }
	std::cout<<"������������allocation\n";
    for(i=0; i<process; i++)
    {
        for(j=0; j<resource; j++)
        {
           std::cin>>allocation[i][j];
        }
    }
    std::cout<<"�����������Դ����available\n";
    for(i=0; i<resource; i++)
    {
        std::cin>>available[i];
    }
}

//��������������
void computeMax(){
	 for(int i=0; i<process; i++)
    {
        for(int j=0; j<resource; j++)
        {
           maxs[i][j]=need[i][j]+allocation[i][j];
        }
    }
	
	
}
//����Ƿ��ڰ�ȫ״̬,�ҵ���ȫ����
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
	std::cout << "�������У�\n";
	std::cout << "        \tallocation         \tneed          \t������ɺ��avilable" << std::endl;
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
					std::cout << '\n' << "����" << i << '\t';
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
			return false;//�����ڰ�ȫ����
		}
	}
	return true;//���ڰ�ȫ����

}
//������Դ
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
//���������Դ�Ƿ񳬹���Ҫ
bool checkNeed(int n,int *reque){
	for(int i=0;i<resource;i++){
		if(reque[i]>need[n][i]) {
			return false;
		   
		}
	}
	return true;
	
}
//���������Դ�Ƿ񳬹�������Դ
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
	std::cout << "��������������ԣ�����Ӱ�죨��q�˳���:\n";
	resourceInput();
	computeMax();
	if (isSafe()) {
		std::cout << "t0ʱ�̰�ȫ\n";
	}
	else std::cout << "t0ʱ�̲���ȫ\n";
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
		
		if (_kbhit()) // ����а���������
		{
			if (_getch() == 'q') //���������q��������ѭ��
			{
				break;
			}

		}
		
		
		std::cout<<"������Ҫ������Դ�Ľ��̱��:\n";
		int number;
		std::cin>>number;
		std::cout<<"����������Դ����:\n";
		//int *ask_for=new int[resource];
		for(int i=0;i<resource;i++)
		{
			std::cin>>request[number][i];
		}
		std::cout<<"��������...\n";
		int temp = Allocate(number, request[number]);

		if (temp==1)
		{
			if (isSafe())
			
				std::cout << "����" << number << "������Դ�ǰ�ȫ��\n";
			else
				std::cout << "����" << number << "������Դ�ǲ���ȫ��\n";

			print();
		}
		else if(temp==2)
			std::cout << "����" << number << "������Դ�����������\n";
		else if (temp == 3)
			std::cout << "����" << number << "������Դ����ϵͳ������Դ\n";
		
		
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
	
	std::cout << "�����Ƕ������ԣ���ǰ���������Ӱ��,��q�˳�:\n";
	resourceInput();
	computeMax();
	if (isSafe()) {
		std::cout << "t0ʱ�̰�ȫ\n";
	}
	else std::cout << "t0ʱ�̲���ȫ\n";

	std::cout << "������Ҫ������Դ�Ľ��̱��:\n";
	while(1){
		
		if (_kbhit()) // ����а���������
		{
			if (_getch() == 'q') //���������q��������ѭ��
			{
				break;
			}

		}
		std::cout << "����������̱��:\n";
		int number;
		std::cin>>number;
		std::cout<<"����������Դ����:\n";
		//int *ask_for=new int[resource];
		for(int i=0;i<resource;i++)
		{
			std::cin>>request[number][i];
		}
		std::cout<<"��������...\n";
		int temp=Allocate(number,request[number]);
		if (temp == 1)
		{
			if (isSafe())
				std::cout << "����" << number << "������Դ�ǰ�ȫ��\n";
			else
				std::cout << "����" << number << "������Դ�ǲ���ȫ��\n";
			print();
		}
		else if (temp == 2)
			std::cout << "����" << number << "������Դ�����������\n";
		else if (temp == 3)
			std::cout << "����" << number << "������Դ����ϵͳ������Դ\n";
	}
	
}

void print() {
	std::cout << " ��ǰ����״��\n";
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
