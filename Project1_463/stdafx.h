#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#ifdef _DEBUG
	#pragma comment(lib, "HTMLParser_debug_x64.lib")
	#pragma comment(lib, "HTMLParser_debug_win32.lib")
#else
	#pragma comment(lib, "HTMLParser_release_x64.lib")
	#pragma comment(lib, "HTMLParser_release_win32.lib")
#endif
using namespace std;