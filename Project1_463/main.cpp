/*
	Created by: Austin Fisher
	Class:	    CSCE 463
	Section:    500
	File:       main.cpp
*/

#include "Socket.h"
#include "Uniqueness.h"
#include "HTMLParserBase.h"
#include "stdafx.h"

#include <fstream>


using namespace std;
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
UrlObj parse_url(string url);
bool connect_verify(UrlObj &input,Socket &sock, string method, int valid_code, int maximum_size, int maximum_time);

int main(int argc, char **argv)
{
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
	if (argc == 2) {
		url = string(argv[1]);
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
	if (argc == 3) {
		if (strcmp(argv[1],"1") != 0) {
			printf("USAGE: Currently only 1 thread is supported.\n");
			cin.ignore();
			return 2;
		}
		try {
			thread_count = stoi(argv[1]);
		}
		catch (...) {
			printf("USAGE: winsock.exe <# of threads> <File_name of file listing urls>\n");
			return 3;
		}

		// Open and load file into memory, set number of threads to run.
		ifstream file(argv[2]);
		if (!file.is_open()) {
			printf("Unable to open file.\n");
			return 4;
		}
		string line;
		while(getline(file,line))
		{
			// Add lines in file to url database
			urls.push_back(line);
		}
	}
	Socket sock;
	Uniqueness checkUnique;
	int nextUrl = 0;
	do {
		if (argc == 3) {
			if (nextUrl == urls.size()) {
				// IF you have processed all strings, terminate
				break;
			}
			url = urls.at(nextUrl++);
			try {
				sock.Refresh();
			}
			catch (...) {
				// Sock throws an error if unable to allcoate memory
				break;
			}
		}

		UrlObj inputUrl;
		printf("URL: %s\n", url.c_str());
		printf("    Parsing URL ...");
		inputUrl = parse_url(url);
		if (inputUrl.invalid) {
			if (argc == 2) {
				break;
			}
			else {
				continue;
			}
		}
		if (argc == 2) {
			printf(" host %s, port %i, request %s\n", inputUrl.host.c_str(), inputUrl.port, inputUrl.request().c_str());
		}
		else{
			printf(" host %s, port %i\n", inputUrl.host.c_str(), inputUrl.port);
		}
		if (argc == 3) {
			// Check if host is unique
			printf("    Checking uniqueness of host...");
			if (checkUnique.host(inputUrl.host)) {
				// Host is unique
				printf(" passed.\n");
			}
			else {
				// Non-unique host, terminate so you don't spam it
				printf(" failed.\n");
				continue;
			}
		}
		printf("    Doing DNS... ");

		if (!sock.Open()) {
			printf("Failed to open socket!\n");
			if (argc == 2) {
				break;
			}
			else {
				continue;
			}
		}

		// Transform DNS
		DWORD t = timeGetTime();

		if (!sock.DNS(inputUrl.host.c_str())) {
			printf("Failed to lookup DNS! \n");
			if (argc == 2) {
				break;
			}
			else {
				continue;
			}
		}
		char *ip_addr_str = inet_ntoa(sock.server.sin_addr);
		printf("done in %d ms, found %s\n", timeGetTime() - t, ip_addr_str);
		string method="GET";
		// return size -1 indicates no maximum
		int good_status_code = 200, max_return_size = -1;
		if (argc == 3) {
			printf("    Checking uniqueness of ip... ");
			// verify uniqueness of DNS
			if (checkUnique.IP(inet_addr(ip_addr_str))) {
				printf(" passed.\n");
				method = "HEAD";
				good_status_code = 400;
				max_return_size = 16000;
			}
			else {
				printf(" failed.\n");
				continue;
			}
		}

		if (!connect_verify(inputUrl, sock, method, good_status_code, max_return_size, (argc==3)?10:-1)) {
			if (argc == 3) {
				continue;
			}
			else {
				break;
			}
		}
		if (argc == 3) {
			// connect to download page
			sock.Refresh();
			sock.Open();
			if (!connect_verify(inputUrl, sock, "GET", 200, 2000000, 10)) {
				continue;
			}
		}
		
		printf("  + Parsing page... ");
		// Parse page for links
		t = timeGetTime();
		HTMLParserBase *parser = new HTMLParserBase;
		int nLinks;
		char *linkBuffer = parser->Parse(sock.buf, sock.curPos, argv[1], strlen(argv[1]), &nLinks);

		// check for errors indicated by negative values
		if (nLinks < 0)
			nLinks = 0;

		printf("done in %d ms with %i links\n\n", timeGetTime() - t, nLinks);
		if (argc == 2) {
			break;
		}
	}while (true);
	// get current time; link with winmm.lib
	if (argc == 2) {
		printf("\n======================================\n");
		//printf("%.*s\n",mySock.curPos,mySock.buf);
		auto headers = strstr(sock.buf, "\r\n\r\n");

		printf("%.*s\n", headers - sock.buf, sock.buf);
		/*FILE *fp;
		fopen_s(&fp, string(inputUrl.host + ".html").c_str(), "wb");
		fwrite(mySock.buf, 1, mySock.curPos, fp);
		fclose(fp);*/

	}
	// call cleanup when done with everything and ready to exit program
	WSACleanup();
	cin.ignore();
	return 0;
}

UrlObj parse_url(string url)
{
	UrlObj inputUrl = UrlObj();

	int scheme_end = url.find("://");
	if (scheme_end == string::npos) {
		printf("Could not find '://'! Are you sure you input a valid address?\n");
		inputUrl.invalid = true;
		return inputUrl;
	}
	inputUrl.scheme = url.substr(0, scheme_end);
	if (inputUrl.scheme != "http") {
		printf("Invalid scheme, only 'http' is accepted\n");
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
				printf("Could not convert port to number! Are you sure it is correct?\n");
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

bool connect_verify(UrlObj & input, Socket &sock, string method, int valid_code, int maximum_size, int maximum_time)
{
	// method should be either "GET" or "HEAD"
	
	string con = (method == "GET") ? "page" : "robots";
	string asterisk = (method == "GET") ? "*" : " ";
	printf("  %s Connecting on %s... ", asterisk.c_str(), con.c_str());
	// Connect to page
	DWORD t = timeGetTime();
	// connect to the server on the requested port
	if (!sock.Connect(input.port)) {
		printf("Failed to connect to port!\n");
		return false;
	}
	printf("done in %d ms\n", timeGetTime() - t);
	printf("    Loading... ");
	
	// Download result
	//string get_http = "GET " + inputUrl.request() + " HTTP/1.0\r\nHost: " + inputUrl.host + "\r\nUser-agent: FisherTAMUcrawler/1.1\r\nConnection: close\r\n\r\n";
	string request = (method == "GET") ? input.request() : "/robots.txt";
	string get_http = method + " " + request + " HTTP/1.0\r\nHost: " + input.host + "\r\nUser-agent: FisherTAMUcrawler/1.2\r\nConnection: close\r\n\r\n";
	t = timeGetTime();
	if (!sock.Write(get_http.c_str())) {
		return false;
	}
	if (!sock.Read(maximum_time, maximum_size)) {
		return false;
	}
	printf("done in %d ms with %i bytes\n", timeGetTime() - t, sock.curPos);
	printf("    Verifying header... ");

	// Check header code
	auto status = strstr(sock.buf, "HTTP/");
	if (status == NULL) {
		// Fail out of checking header code
		printf(" irregular location of status code, unable to parse!\n");
		return false;
	}
	int status_code = -1;
	try {
		status_code = stoi(status + 9);
	}
	catch (...) {
		printf("Problem retriving status code from response!\n");
		return false;
	}
	printf("status code %d\n", status_code);
	if (status_code < valid_code || status_code >= valid_code + 100) {
		return false;
	}
	return true;
}
