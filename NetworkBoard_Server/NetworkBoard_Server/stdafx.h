// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <timeapi.h>
// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <stdio.h>
// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#define PORT 25000
#define USER 100

#define WM_NETWORK WM_USER + 1

#include "RingBuffer.h"

// �߰� ����
// PACKET ��Ʈ
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