#include<iostream>
#include<list>
#include<iomanip>
enum FitAlgorithm { FirstFit, BestFit, WorstFit };
const FitAlgorithm SelectedAlgorithm = FirstFit;

class memory
{
public:
	std::string process_name;
	int process_space;
	int memory_space;
	bool valid = true;
public:
	memory(std::string Name, int ProcessSpace)
	{
		this->process_name = Name;
		this->process_space = ProcessSpace;
		this->memory_space = ProcessSpace;
	}

	memory(std::string Name, int ProcessSpace, int MemorySpace)
	{
		this->process_name = Name;
		this->process_space = ProcessSpace;
		this->memory_space = MemorySpace;
	}

	bool operator==(memory& m)
	{
		return (m.process_name == this->process_name) && (m.process_space == this->process_space);
	}

};

class MemoryManage
{
public:
	MemoryManage()
	{
		memory m("Operating System", 40);
		std::cout << "\n-------------------t0:操作系统初始化40K内存-------------------\n";
		allocate(m, SelectedAlgorithm);
	}

	void allocate(memory& m, FitAlgorithm algorithm )
	{
		std::list<memory>::iterator selected_it = memory_list.end();
		for (auto it = memory_list.begin(); it != memory_list.end(); it++)
		{
			if (it->valid == false && it->memory_space >= m.process_space)
			{
				if (algorithm == FirstFit)
				{
					selected_it = it;
					break;
				}
				else if (algorithm == BestFit && (selected_it == memory_list.end() || it->memory_space < selected_it->memory_space))
				{
					selected_it = it;
				}
				else if (algorithm == WorstFit && (selected_it == memory_list.end() || it->memory_space > selected_it->memory_space))
				{
					selected_it = it;
				}
			}
		}

		if (selected_it != memory_list.end())
		{
			selected_it->process_name = m.process_name;
			selected_it->process_space = m.process_space;
			selected_it->valid = true;
		}
		else
		{
			memory_list.push_back(m);
			unallocate_memory_size -= m.memory_space;
		}
	}

	void deallocate(memory& m)
	{
		for (auto it = memory_list.begin(); it != memory_list.end(); it++)
		{
			if (*it == m)
			{
				it->valid = false;
				it->process_name = "NULL";
				it->process_space = 0;

				//前面有空位置可以合并
				if (it != memory_list.begin() && std::prev(it)->valid == false)
				{
					it->memory_space += std::prev(it)->memory_space;
					memory_list.erase(std::prev(it));
				}

				//后面有位置可以合并
				if (std::next(it) != memory_list.end() && std::next(it)->valid == false)
				{
					it->memory_space += std::next(it)->memory_space;
					memory_list.erase(std::next(it));
				}

				//合并完后如果是最后一个内存块，可以直接释放
				if (std::next(it) == memory_list.end())
				{
					unallocate_memory_size += it->memory_space;
					memory_list.erase(it);
				}
				break;
			}
		
	}
		std::cout <<"该进程"<<m.process_name<<"已经释放\n";
}


	void print()
	{
		int i = 0;
		for (auto it : memory_list)
		{
			; std::cout <<"内存块:" << std::left << std::setw(30) << i++;
			; std::cout <<"进程名称:" << std::left << std::setw(30) << it.process_name;
			; std::cout <<"进程实际大小:" << std::left << std::setw(30) << it.process_space;
			; std::cout <<"占用内存大小:" << std::left << std::setw(30) << it.memory_space << std::endl;;
			;
		}
		; std::cout << "内存块剩余空间:" << unallocate_memory_size << std::endl;

	}
private:
	std::list<memory> memory_list;
	int unallocate_memory_size = 640;
};


int main()
{
	MemoryManage management;
	memory A("Process A", 80);
	memory B("Process B", 60);
	memory C("Process C", 100);
	std::cout << "\n-------------------t1:为进程A、B、C分配80K、60K、100K内存空间-------------------\n";
	management.allocate(A,SelectedAlgorithm);
	management.allocate(B, SelectedAlgorithm);
	management.allocate(C, SelectedAlgorithm);
	management.print();

	std::cout << "\n-------------------t2:进程B完成-------------------\n" ;
	management.deallocate(B);
	management.print();


	std::cout << "\n-------------------t3:为进程D分配50K的内存空间-------------------\n" ;
	memory D("Process D", 50);
	management.allocate(D, SelectedAlgorithm);
	management.print();

	std::cout  << "\n-------------------t4:进程A、C完成-------------------\n" ;
	management.deallocate(A);
	management.deallocate(C);
	management.print();


	std::cout << "\n-------------------t5:进程D完成-------------------\n" ;
	management.deallocate(D);
	management.print();
}
