/*
Created by: Austin Fisher
Class:	    CSCE 463
Section:    500
File:       Uniqueness.h
*/
#pragma once
#include "stdafx.h"
#include <unordered_set>

class Uniqueness
{
private:
	std::unordered_set<std::string> seenHosts;
	std::unordered_set<DWORD> seenIps;
	CRITICAL_SECTION ipMutex;
	CRITICAL_SECTION hostMutex;
public:
	bool IP(DWORD ip);
	bool host(std::string host);
	int ipCount(void);
	int hostCount(void);
	Uniqueness();
	~Uniqueness();
};

