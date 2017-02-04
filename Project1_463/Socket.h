#pragma once
#include "stdafx.h"


#define INITIAL_BUF_SIZE 8000      // 8 kb
#define THRESHOLD 250
class Socket
{
public:
	SOCKET sock;                   // socket handle
	char *buf;                     // current buffer
	int allocatedSize;             // bytes allocated for buf
	int curPos;                    // current position in buffer
	struct sockaddr_in server;     // Used for connecting to server
	Socket();
	bool Open(void);
	bool Connect(int port);
	bool Read(int max_time, int max_size);
	bool DNS(const char host[]);
	bool Write(const char message[]);
	void Refresh();
	~Socket();
};

