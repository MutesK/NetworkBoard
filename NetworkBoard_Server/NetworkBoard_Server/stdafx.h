// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <timeapi.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <stdio.h>
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#define PORT 25000
#define USER 100

#define WM_NETWORK WM_USER + 1

#include "RingBuffer.h"

// 추가 변수
// PACKET 파트
#pragma pack(push, 1)
struct st_PACKET
{
	unsigned short len;  // Header
	int iStartX;
	int iStartY;
	int iEndX;
	int iEndY;
};
#pragma pack(pop)

struct st_USER_QUEUE
{
	SOCKET sock;
	bool bSendFlag;
	CRingBuffer SendQ;
	CRingBuffer RecvQ;
};