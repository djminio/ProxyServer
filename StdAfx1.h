#pragma once


//proxyserver config
#ifndef WINVER
#define _WIN32_WINNT 0x0A00
#endif

#pragma warning(disable: 4996)

#include <afx.h>
#include <ole2.h>
#include <initguid.h>
#include <windows.h>
#include <winsock2.h>
// 공유 라이브러리
#pragma comment (lib, "../Library/HSEL.lib")
#include "../Shared/HSEL.h"
#pragma comment (lib, "../Library/Shared.lib")
#include "../Shared/Shared.h"

#pragma once

//#define WINVER 0x0500

//#include <afx.h>
#ifdef _USE_WINSOCK
	#include <winsock2.h>
#endif

#include <windows.h>
#include <stdio.h>
#include <ole2.h>
#include <initguid.h>

#ifdef _USE_WINSOCK
	#include <mstcpip.h>
#endif



#define GUID_SIZE 128
#define MAX_STRING_LENGTH 256
typedef void**	PPVOID;