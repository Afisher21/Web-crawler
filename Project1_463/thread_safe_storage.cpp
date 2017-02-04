/*
Created by: Austin Fisher
Class:	    CSCE 463
Section:    500
File:       thread_safe_storage.cpp
*/

#include "thread_safe_storage.h"



void thread_safe_storage::add(string input, bool last_value)
{
	EnterCriticalSection(mutex);
	data.push_back(input);
	if (last_value)
		finished = true;
	LeaveCriticalSection(mutex);
	
}

string thread_safe_storage::read(void)
{
	EnterCriticalSection(mutex);
	if (finished) {
		LeaveCriticalSection(mutex);
		return NULL;
	}
	if (next_read >= data.size()) {
		// Need to try again since it was stuck reading
		throw;
	}
	string result = data[next_read];
	next_read++;
	LeaveCriticalSection(mutex);
	return result;
}

thread_safe_storage::thread_safe_storage()
{
	data = vector<string>();
	next_read = 0;
	InitializeCriticalSection(mutex);
	finished = false;
}


thread_safe_storage::~thread_safe_storage()
{
}
