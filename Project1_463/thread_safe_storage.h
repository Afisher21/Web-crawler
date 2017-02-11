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
	int nextRead;
	CRITICAL_SECTION mutex;
public:
	bool finished;
	void add(string input);
	int read(string &bind);
	int size(void);
	int remaining(void);
	thread_safe_storage();
	~thread_safe_storage();
};

