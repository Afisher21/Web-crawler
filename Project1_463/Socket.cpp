#include "Socket.h"
#include "stdafx.h"



Socket::Socket()
{
	buf = new char[INITIAL_BUF_SIZE];
	if (buf == nullptr) {
		printf("Error allocating memory for Socket buffer!\n");
		throw;
	}
	allocatedSize = INITIAL_BUF_SIZE;
	curPos = 0;
	sock = INVALID_SOCKET;
}

bool Socket::Open(void)
{
	// open a TCP socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("socket() generated error %d\n", WSAGetLastError());
		WSACleanup();
		return false;
	}
	return true;
}

bool Socket::Connect(int port)
{
	// setup the port # and protocol type
	server.sin_family = AF_INET;
	server.sin_port = htons(port);		// host-to-network flips the byte order
	
	if (connect(sock, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		printf("failed with %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

bool Socket::Read(int max_time, int max_size)
{
	// set timeout to 10 seconds
	timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	fd_set fd;
	int ret; // Return value from select
	DWORD startTime = timeGetTime();
	while (true)
	{
		if (max_time != -1) {
			// *1000 to convert from seconds to milliseconds
			if (timeGetTime() - startTime > max_time * 1000) {
				printf("timed out.\n");
				return false;
			}
		}
		if (max_size != -1) {
			if (curPos > max_size) {
				printf("exceeded maximum file size.\n");
				return false;
			}
		}
		FD_ZERO(&fd);
		FD_SET(sock, &fd);
		// wait to see if socket has any data (see MSDN)
		if (ret = select(0, &fd, 0, 0, &timeout) > 0)
		{
			// new data available; now read the next segment
			// Available size -1 so there is room for a null termination.
			int bytes = recv(sock, buf + curPos, allocatedSize - curPos -1 , 0);
			if (bytes == SOCKET_ERROR) {
				printf("failed with %d on recv\n", WSAGetLastError());
				return false;
			}
			if (bytes == 0) {
				//memset(buf + curPos, ' ', allocatedSize - curPos);
				buf[curPos] = NULL;
				return true; // normal completion
			}
			curPos += bytes; // adjust where the next recv goes
			if (allocatedSize - curPos < THRESHOLD) {
				// resize buffer; besides STL, you can use
				// realloc(), HeapReAlloc(), or memcpy the buffer
				// into a bigger array
				allocatedSize += allocatedSize;
				buf = (char*) realloc(buf, allocatedSize);
			}
		}
		else if (ret == -1) {
			// print WSAGetLastError()
			printf("failed with %d\n", WSAGetLastError());
			break;
		}
		else {
			// report timeout
			printf("failed due to time out\n");
			break;
		}
	}
	return false;
}

bool Socket::DNS(const char host[])
{
	// structure used in DNS lookups
	struct hostent *remote;
	DWORD IP = inet_addr(host);
	if (IP == INADDR_NONE)
	{
		// if not a valid IP, then do a DNS lookup
		if ((remote = gethostbyname(host)) == NULL)
		{
			printf("failed with invalid string: neither FQDN, nor IP address\n");
			return false;
		}
		else // take the first IP address and copy into sin_addr
			memcpy((char *)&(server.sin_addr), remote->h_addr, remote->h_length);
	}
	else
	{
		// if a valid IP, directly drop its binary version into sin_addr
		server.sin_addr.S_un.S_addr = IP;
	}
	return true;
}


bool Socket::Write(const char message[])
{
	int res = send(sock, message, strlen(message), 0);
	if (res == SOCKET_ERROR) {
		printf("failed with %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void Socket::Refresh()
{
	// Wipe old buffer, resize if too large
	if (allocatedSize > 32000) {
		delete[] buf;
		buf = new char[INITIAL_BUF_SIZE];
		if (buf == nullptr) {
			printf("Unable to allocate memory for socket buffer!!!\n");
			throw;
		}
		allocatedSize = INITIAL_BUF_SIZE;
	}
	curPos = 0;
	memset(buf, 0, allocatedSize);
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
	}
}

Socket::~Socket()
{
	closesocket(sock);
}