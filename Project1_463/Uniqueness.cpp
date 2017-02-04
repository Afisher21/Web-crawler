#include "Uniqueness.h"
#include "stdafx.h"



bool Uniqueness::IP(DWORD ip)
{
	EnterCriticalSection(&ip_mutex);
	size_t prevSize = seenIps.size();
	seenIps.insert(ip);
	size_t newSize = seenIps.size();
	LeaveCriticalSection(&ip_mutex);
	if (newSize > prevSize)
		return true;
	return false;
}

bool Uniqueness::host(std::string host)
{
	EnterCriticalSection(&host_mutex);
	size_t prevSize = seenHosts.size();
	seenHosts.insert(host);
	size_t newSize = seenHosts.size();
	LeaveCriticalSection(&host_mutex);
	if (newSize > prevSize)
		return true;
	return false;
}

Uniqueness::Uniqueness()
{
	seenHosts = std::unordered_set<std::string>();
	seenIps = std::unordered_set<DWORD>();
	InitializeCriticalSection(&ip_mutex);
	InitializeCriticalSection(&host_mutex);
}


Uniqueness::~Uniqueness()
{
	DeleteCriticalSection(&ip_mutex);
	DeleteCriticalSection(&host_mutex);
}
