/*
	Created by: Austin Fisher
	Class:	    CSCE 463
	Section:    500
	File:       main.cpp
*/

#include "Socket.h"
#include "Uniqueness.h"
#include "HTMLParserBase.h"
#include "thread_safe_storage.h"
#include "CommonStructs.h"
#include "stdafx.h"
#include <fstream>


UrlObj parse_url(string url);
bool connect_verify(UrlObj &input, Socket &sock, string method, int &valid_code, int maximum_size, int maximum_time);
UINT status_thread(LPVOID pParam);
UINT url_producer_thread(LPVOID pParam);
UINT url_crawler_thread(LPVOID pParam);

int main(int argc, char **argv)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	DWORD t_total = timeGetTime();
	WSADATA wsaData;
	string url = "";
	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	if (argc > 3 || argc < 2) {
		printf("Unknown parameters!\n");
		printf("USAGE:\n  winsock.exe scheme://host[:port][/path][?query][#fragment]\n");
		printf("OR\n");
		printf("winsock.exe <# of threads> <File_name of file listing urls>\n");
		return 1;
	}
	int thread_count = 1;
	vector<string> urls;
	try {
		thread_count = stoi(argv[1]);
	}
	catch (...) {
		printf("USAGE: winsock.exe <# of threads> <File_name of file listing urls>\n");
		return 3;
	}
	Parameters p;
	// Open and load file into memory, set number of threads to run.
	ifstream file(argv[2]);
	if (!file.is_open()) {
		printf("Unable to open file.\n");
		return 4;
	}
	string line;
	while (getline(file, line))
	{
		// Add lines in file to url database
		p.urls.add(line);
	}
	Socket sock;
	Uniqueness checkUnique;
	HANDLE *handles = new HANDLE[thread_count + 1];  // +1 for stats thread
	handles[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)status_thread, &p, 0, NULL);
	for (int i = 1; i <= thread_count; ++i) {
		handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)url_crawler_thread, &p, 0, NULL);
	}

	// make sure this thread hangs here until the other three quit; otherwise, the program will terminate prematurely
	for (int i = 0; i <= thread_count; i++)
	{
		WaitForSingleObject(handles[i], INFINITE);
		CloseHandle(handles[i]);
	}

	WSACleanup();
	cin.ignore();
	return 0;
}

UrlObj parse_url(string url)
{
	UrlObj inputUrl = UrlObj();

	int scheme_end = url.find("://");
	if (scheme_end == string::npos) {
		inputUrl.invalid = true;
		return inputUrl;
	}
	inputUrl.scheme = url.substr(0, scheme_end);
	if (inputUrl.scheme != "http") {
		inputUrl.invalid = true;
		return inputUrl;
	}
	// Remove the processed ends of the URL
	url = url.substr(scheme_end + 3);
	int fragment_start = url.find("#");
	if (fragment_start != string::npos)
		url = url.substr(0, fragment_start);
	// Look for port, path, query
	int port_start = url.find(":");
	int path_start = url.find("/");
	int query_start = url.find("?");
	if (query_start != string::npos && path_start > query_start)
		path_start = string::npos;
	if (path_start != string::npos && port_start > path_start
		|| query_start != string::npos && port_start > query_start)
		port_start = string::npos;

	// Host is whatever is before port. If no port, whatever is before path, ...
	if (port_start != string::npos)
		inputUrl.host = url.substr(0, port_start);
	else if (path_start != string::npos)
		inputUrl.host = url.substr(0, path_start);
	else if (query_start != string::npos)
		inputUrl.host = url.substr(0, query_start);
	else
		inputUrl.host = url;
	if (port_start != string::npos)
	{
		// There is a port. Ends at either '/', '?', or end of input
		string port;
		if (path_start == string::npos && query_start == string::npos)
			port = url.substr(port_start + 1);
		else if (path_start != string::npos)
			port = url.substr(port_start + 1, path_start - port_start - 1);
		else if (query_start != string::npos)
			port = url.substr(port_start + 1, query_start - port_start - 1);
		// Convert port to int
		if (port.size() != 0) {
			try {
				inputUrl.port = stoi(port);
			}
			catch (...) {
				inputUrl.invalid = true;
				return inputUrl;
			}
		}
	}
	if (path_start != string::npos) {
		// There is a path. Ends at either '?', or end of input
		if (query_start == string::npos)
			inputUrl.path = url.substr(path_start);
		else if (query_start != string::npos) {
			inputUrl.path = url.substr(path_start, query_start - path_start);
		}
	}
	if (query_start != string::npos) {
		inputUrl.query = url.substr(query_start);
	}

	return inputUrl;
}

bool connect_verify(UrlObj & input, Socket &sock, string method, int &valid_code, int maximum_size, int maximum_time)
{
	// connect to the server on the requested port
	if (!sock.Connect(input.port)) {
		valid_code = -1;
		return false;
	}

	// Download result
	string request = (method == "GET") ? input.request() : "/robots.txt";
	string get_http = method + " " + request + " HTTP/1.0\r\nHost: " + input.host + "\r\nUser-agent: FisherTAMUcrawler/1.2\r\nConnection: close\r\n\r\n";
	if (!sock.Write(get_http.c_str())) {
		return false;
	}
	if (!sock.Read(maximum_time, maximum_size)) {
		return false;
	}

	// Check header code
	auto status = strstr(sock.buf, "HTTP/");
	if (status == NULL) {
		// Fail out of checking header code
		valid_code = 0;
		return false;
	}
	int status_code = -1;
	try {
		status_code = stoi(status + 9);
	}
	catch (...) {
		valid_code = 0;
		return false;
	}
	if (status_code < valid_code || status_code >= valid_code + 100) {
		valid_code = status_code;
		return false;
	}
	return true;
}

UINT status_thread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);
	/// Expected output
	//
	//[  6] 500 Q 992142 E 7862 H 1790 D 1776 I 1264 R 544 C 190 L 5K
	//		*** crawling 87.5 pps @ 12.3 Mbps
	long prevDownloaded = 0;
	int prevExtractedPages = 0;
	DWORD prevTime = 0;
	DWORD startTime = timeGetTime();
	while (!p->urls.finished || p->threadCount()>0) {
		// It takes a few milliseconds for the above code to run, so sleep until the next wakeup would occur at 2 seconds
		// This does not account for timing errors like the computer waking the program after 2050 ms when 2000 was requested
		DWORD sleepTime = 2000;
		Sleep(sleepTime);
		/// Variables:
		/*
		First column is elapsed time in seconds, 3 character alignment
		Q: current size of the pending queue
		E: number of extracted URLs from the queue
		H: number of URLs that have passed host uniqueness
		D: number of successful DNS lookups
		I: number of URLs that have passed IP uniqueness
		R: number of URLs that have passed robots checks
		C: number of successfully crawled URLs(those with a valid HTTP code)
		L: total links found
		*/
		double wakeUpTime = timeGetTime();
		double timeSinceInitialized = (wakeUpTime - startTime) / 1000;
		double deltaSeconds = timeSinceInitialized - prevTime; // Amount of seconds since last wakeup
		int remaining = p->urls.remaining();
		long downloadedBytes = p->downloadedBytes();
		long bps = (downloadedBytes - prevDownloaded)/ deltaSeconds; // bytes downloaded since last check
		long _Mbps = (bps * 8) / 1000000; // convert MB to b
		int extractedPages = p->totalPages();
		double pagesPerSecond = (deltaSeconds == 0)?0:(extractedPages - prevExtractedPages) / deltaSeconds;
		printf("[%3.0f]  %ld  Q %6d  E %7d  H %6d  D %6ld  I %5d  R %5ld  C %5ld L %4ld\n\t*** crawling %.1d pps @ %.1ld Mbps\n",
			timeSinceInitialized,								// [%3f]   Elapsed time
			p->threadCount(),									//		   Number of threads currently running
			remaining,											// Q	   Size of pending queue
			extractedPages,										// E	   Number of extracted URLs from queue
			p->unique.hostCount(),								// H	   Number of unique hosts so far
			p->succesfulDnsCount(),								// D       Number of successful DNS lookups
			p->unique.ipCount(),								// I	   Number of unique IP addresses
			p->succesfulRobotCount(),							// R	   Number of URLs without robots.txt
			p->totalTwoHundredCount(),							// C	   Number of successfully crawled URLs
			p->totalLinkCount(),								// L	   Total number of links found
			pagesPerSecond,										// pps
			_Mbps												// bps
			);
		prevDownloaded = downloadedBytes;
		prevExtractedPages = extractedPages;
		prevTime = timeSinceInitialized;
	}

	// Print final stats
	DWORD finalTime = (timeGetTime() - startTime)/1000;
	DWORD rate = p->urls.size() / finalTime;
	printf("Total elapsed time: %.2ds\n", finalTime);
	printf("Extracted %d URLs @ %.1ld/s\n",p->urls.size(), rate);
	rate = p->succesfulDnsCount() / finalTime;
	printf("Looked up %d DNS names @ %.1d/s\n", p->succesfulDnsCount(), rate);
	rate = p->succesfulRobotCount() / finalTime;
	printf("Downloaded %d robots @ %.1ld/s\n", p->succesfulRobotCount(), rate);
	rate = p->totalPages() / finalTime;
	printf("Crawled %d pages @ %.1ld/s\n", p->totalPages(), rate);
	rate = p->totalLinkCount() / finalTime;
	printf("Parsed %d links @ %.1ld/s\n", p->totalLinkCount(), rate);
	printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d\n", p->totalTwoHundredCount(),
		p->totalThreeHundredCount(), p->totalFourHundredCount(),
		p->totalFiveHundredCount(),p->totalOtherCount());

	return 0;
}

UINT url_producer_thread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);
	return 0;
}

UINT url_crawler_thread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);
	p->incrementThreadCount();
	HTMLParserBase *parser = new HTMLParserBase;
	Socket sock;
	do {
		sock.Refresh();
		string url;
		if (p->urls.read(url) == -1) {
			// No more urls to read!
			break;
		}
		UrlObj inputUrl;
		inputUrl = parse_url(url);
		if (inputUrl.invalid) {
			// If failed to parse URL, start over and grab a new one
			continue;
		}
		if (!p->unique.host(inputUrl.host)) {
			continue;
		}
		if (!sock.Open()) {
			continue;
		}

		// Transform DNS

		if (!sock.DNS(inputUrl.host.c_str())) {
			continue;
		}
		p->incrementDns();
		// verify uniqueness of DNS
		char *ip_addr_str = inet_ntoa(sock.server.sin_addr);
		if (!p->unique.IP(inet_addr(ip_addr_str))) {
			continue;
		}
		string method = "HEAD";
		int good_status_code = 400, max_return_size = 16000;
		if (!connect_verify(inputUrl, sock, method, good_status_code, max_return_size, 10)) {
			continue;
		}
		p->incrementRobots();
		// connect to download page
		sock.Refresh();
		sock.Open();
		int returnCode = 200;
		if (!connect_verify(inputUrl, sock, "GET", returnCode, 2000000, 10)) {
			if (returnCode >= 300 && returnCode < 400)
				p->incrementThreeXX();
			else if (returnCode >= 400 && returnCode < 500)
				p->incrementFourXX();
			else if (returnCode >= 500 && returnCode < 600)
				p->incrementFiveXX();
			else if (returnCode > 0 )
				p->incrementOther();
			p->increaseDownloadCount(sock.curPos);
			continue;
		}
		p->incrementTwoXX();
		p->increaseDownloadCount(sock.curPos);
		// Parse page for links
		int nLinks;
		char *linkBuffer = parser->Parse(sock.buf, sock.curPos, _strdup(url.c_str()), url.size(), &nLinks);

		// check for errors indicated by negative values
		if (nLinks < 0)
			nLinks = 0;
		long linkCount = (long)nLinks;
		p->addLink(linkCount);
		//free(linkBuffer);
	} while (true);
	delete parser;


	p->decrementThreadCount();
	return 0;
}
