// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#pragma comment(lib, "winmm.lib")
#include <windows.h>
#include <timeapi.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>

// 디버그용 


// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <conio.h>
#include <stdio.h>
#include "RingBuffer.h"
#pragma comment(lib, "ws2_32")


#define PORT 25000

#define WM_NETWORK WM_USER + 1

#pragma pack(push, 1)
struct st_Packet
{
	unsigned short len;  // Header
	int iStartX;
	int iStartY;
	int iEndX;
	int iEndY;
};
#pragma pack(pop)