/*
Created by: Austin Fisher
Class:	    CSCE 463
Section:    500
File:       thread_safe_storage.h
*/
#pragma once
#include "stdafx.h"
class thread_safe_storage
{
private:
	vector<string> data;
	int next_read;
	bool finished;
	LPCRITICAL_SECTION mutex;
public:
	void add(string input, bool last_value);
	string read(void);
	thread_safe_storage();
	~thread_safe_storage();
};

