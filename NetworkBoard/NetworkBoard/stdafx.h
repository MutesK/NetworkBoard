// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#pragma comment(lib, "winmm.lib")
#include <windows.h>
#include <timeapi.h>
// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>

// ����׿� 


// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
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