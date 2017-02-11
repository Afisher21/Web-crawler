/*
Created by: Austin Fisher
Class:	    CSCE 463
Section:    500
File:       CommonStructs.h
*/
#pragma once
#include "stdafx.h"
struct UrlObj {
	bool invalid = false;
	string scheme;
	string host;
	int port = 80;
	string path = "/"; // Initial value if no path provided
	string query;
	string request() {
		return path + query;
	}
};

// this class is passed to all threads, acts as shared memory
struct Parameters {
private:
	long succesfulDns = 0;
	long hasRobots = 0;
	long twoHundredCount = 0;
	long threeHundredCount = 0;
	long fourHundredCount = 0;
	long fiveHundredCount = 0;
	long otherCount = 0;
	long linkCount = 0;
	long activeThreadCount = 0;
	long totalDownloaded = 0;
public:
	thread_safe_storage urls;		// Used to give all threads a common url pool to pull from safely
	Uniqueness unique;				// Used to ensure Host & IP have not been accessed before by a different thread
	void incrementThreadCount(void) {
		InterlockedIncrement(&activeThreadCount);
	}
	void decrementThreadCount(void) {
		InterlockedDecrement(&activeThreadCount);
	}
	long threadCount(void) {
		return activeThreadCount;
	}
	void increaseDownloadCount(long downloaded) {
		InterlockedAdd(&totalDownloaded, downloaded);
	}
	long downloadedBytes(void) {
		return totalDownloaded;
	}
	void incrementRobots(void) {
		InterlockedIncrement(&hasRobots);
	};
	void incrementDns(void) {
		InterlockedIncrement(&succesfulDns);
	};
	void incrementTwoXX(void) {
		InterlockedIncrement(&twoHundredCount);
	};
	void incrementThreeXX(void) {
		InterlockedIncrement(&threeHundredCount);
	};
	void incrementFourXX(void) {
		InterlockedIncrement(&fourHundredCount);
	};
	void incrementFiveXX(void) {
		InterlockedIncrement(&fiveHundredCount);
	};
	void incrementOther(void) {
		InterlockedIncrement(&otherCount);
	}
	void addLink(long link) {
		InterlockedAdd(&linkCount, link);
	}
	long totalLinkCount() {
		return linkCount;
	}
	long totalTwoHundredCount() {
		return twoHundredCount;
	};
	long totalThreeHundredCount() {
		return threeHundredCount;
	};
	long totalFourHundredCount() {
		return fourHundredCount;
	};
	long totalFiveHundredCount() {
		return fiveHundredCount;
	};
	long totalPages() {
		return twoHundredCount + threeHundredCount + fourHundredCount + fiveHundredCount + otherCount;
	}
	long totalOtherCount() {
		return otherCount;
	}
	long succesfulDnsCount() {
		return succesfulDns;
	};
	long succesfulRobotCount() {
		return hasRobots;
	};
};