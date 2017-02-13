/*
Created by: Austin Fisher
Class:	    CSCE 463
Section:    500
File:       thread_safe_storage.cpp
*/

#include "thread_safe_storage.h"



void thread_safe_storage::add(string input)
{
	//EnterCriticalSection(&mutex);
	data.push_back(input);
	//LeaveCriticalSection(&mutex);
	
}

int thread_safe_storage::read(string &bind)
{
	EnterCriticalSection(&mutex);
	if (finished) {
		LeaveCriticalSection(&mutex);
		return -1;
	}
	if (nextRead == data.size()) {
		finished = true;
		LeaveCriticalSection(&mutex);
		nextRead++;
		return -1;
	}
	bind = data[nextRead];
	nextRead++;
	LeaveCriticalSection(&mutex);
	return 0;
}

int thread_safe_storage::size(void)
{
	return data.size();
}

int thread_safe_storage::remaining(void)
{
	EnterCriticalSection(&mutex);
	int read = nextRead==0?0:nextRead-1;
	LeaveCriticalSection(&mutex);

	return data.size() - read;
}

thread_safe_storage::thread_safe_storage()
{
	data = vector<string>();
	nextRead = 0;
	InitializeCriticalSection(&mutex);
	finished = false;
}


thread_safe_storage::~thread_safe_storage()
{
	DeleteCriticalSection(&mutex);
}
