#include "thread_pool.h"
#include<iostream>

using namespace std;

void print_message(vector<void*> vec){
//	void* p = vec[0];
	cout<<"this is "<<endl;
//	sleep(1);
}


int main(){
	CThreadPool threadpool;
	threadpool.run(5);

	int N = 20;
//	while(1){
//	char* p = new char[100];
//	strcpy(p,"this is pthread");
	
	for(int i = 1;i<15;i++){
//		strcpy(p,"this is pthread");
		vector<void*> vec;
//		vec.push_back((void*)(p));
		CTask *task = new CTask(&print_message,vec);
		threadpool.add_task(task);
//		p[0]++;
//		sleep(1);
	}
//	delete p;
//	}

	
//	threadpool.stop();

	return 0;

}


		
