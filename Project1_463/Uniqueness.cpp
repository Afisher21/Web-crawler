/*
Created by: Austin Fisher
Class:	    CSCE 463
Section:    500
File:       Uniqueness.cpp
*/
#include "Uniqueness.h"
#include "stdafx.h"



bool Uniqueness::IP(DWORD ip)
{
	EnterCriticalSection(&ipMutex);
	size_t prevSize = seenIps.size();
	seenIps.insert(ip);
	size_t newSize = seenIps.size();
	LeaveCriticalSection(&ipMutex);
	if (newSize > prevSize)
		return true;
	return false;
}

bool Uniqueness::host(std::string host)
{
	EnterCriticalSection(&hostMutex);
	size_t prevSize = seenHosts.size();
	seenHosts.insert(host);
	size_t newSize = seenHosts.size();
	LeaveCriticalSection(&hostMutex);
	if (newSize > prevSize)
		return true;
	return false;
}

int Uniqueness::ipCount(void)
{
	EnterCriticalSection(&ipMutex);
	int count = seenIps.size();
	LeaveCriticalSection(&ipMutex);
	return count;
}

int Uniqueness::hostCount(void)
{
	EnterCriticalSection(&hostMutex);
	int count = seenHosts.size();
	LeaveCriticalSection(&hostMutex);
	return count;
}

Uniqueness::Uniqueness()
{
	seenHosts = std::unordered_set<std::string>();
	seenIps = std::unordered_set<DWORD>();
	InitializeCriticalSection(&ipMutex);
	InitializeCriticalSection(&hostMutex);
}


Uniqueness::~Uniqueness()
{
	DeleteCriticalSection(&ipMutex);
	DeleteCriticalSection(&hostMutex);
}
