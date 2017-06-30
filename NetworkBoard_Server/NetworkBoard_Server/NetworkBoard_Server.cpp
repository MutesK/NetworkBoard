// NetworkBoard_Server.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "NetworkBoard_Server.h"


#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND hMDlg;
HWND g_hWnd;
HWND list;

int g_UserCount = 0;
int g_UserF = 0;
st_USER_QUEUE Queue[USER];
bool bSendFlag = false;

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK	DialogProc(HWND, UINT, WPARAM, LPARAM);

// 네트워크 함수
void RemoveSocket(SOCKET sock);
void AddClientSocket(SOCKET sock);
void RecvClientEvent(SOCKET sock);
void SendDraw(st_PACKET *Packet);
// WSAWOULDBLOCK 문제 반응 함수
void SendClientEvent(SOCKET sock);
void SendAll();
void PacketProc(CRingBuffer *pRecv);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	timeBeginPeriod(1);

    // TODO: 여기에 코드를 입력합니다.
	if (AllocConsole()) {
		freopen("CONIN$", "rb", stdin);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);
	}

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NETWORKBOARD_SERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);



    // 응용 프로그램 초기화를 수행합니다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	int err;
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// Socket
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
	{
		return 1;
	}

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	InetPton(AF_INET, L"127.0.0.1", &serveraddr.sin_addr);
	serveraddr.sin_port = htons(PORT);
	err = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (err == SOCKET_ERROR)
	{
		return 1;
	}

	// listen()
	err = listen(listen_sock, SOMAXCONN);
	if (err == SOCKET_ERROR)
	{
		return 1;
	}

	hMDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), g_hWnd, DialogProc);
	ShowWindow(hMDlg, SW_SHOW);

	// WSAAsyncSelect
	err = WSAAsyncSelect(listen_sock, hMDlg, WM_NETWORK, FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE);

	printf("서버 대기중\n");
    MSG msg;

    // 기본 메시지 루프입니다.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
    }

	WSACleanup();
    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NETWORKBOARD_SERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_NETWORKBOARD_SERVER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;

  // ShowWindow(hWnd, nCmdShow);
  // UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		list = GetDlgItem(hWnd, IDC_LIST1);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			DestroyWindow(hMDlg);
			EndDialog(hWnd, IDCANCEL);
			hMDlg = NULL;
			return TRUE;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_NETWORK:
		if (WSAGETSELECTERROR(lParam))
		{
			// 해당 소켓 제거
			printf("Error Socket !!");
			RemoveSocket(wParam);
			return 0;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			// 유저 추가
			AddClientSocket(wParam);
			break;
		case FD_READ:
			RecvClientEvent(wParam); // 해당 소켓
			break;
		case FD_WRITE:
			SendClientEvent(wParam);
			break;
		case FD_CLOSE:
			// 해당 소켓 제거
			RemoveSocket(wParam);
			break;
		}

	}
	return 0;
}


void RemoveSocket(SOCKET sock)
{
	for (int i = 0; i < g_UserF; i++)
	{
		if (sock == Queue[i].sock)
		{
			st_USER_QUEUE *dP = &Queue[i];
			SOCKADDR_IN clientaddr;
			int addrlen = sizeof(clientaddr);

			getpeername(sock, (SOCKADDR *)&clientaddr, &addrlen);
			g_UserCount--;
			printf("\n Debug : Client 접속 해제 : IP 주소 : %s \n", inet_ntoa(clientaddr.sin_addr));
			SendMessage(list, LB_ADDSTRING, 0, (LPARAM)L"Client 접속 해제");
			closesocket(dP->sock);
			dP->sock = INVALID_SOCKET;

			dP->RecvQ.ClearBuffer();
			dP->SendQ.ClearBuffer();
		}
	}
}

void AddClientSocket(SOCKET listen_sock)
{
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	st_USER_QUEUE *dP = NULL;

	client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET)
	{
		return;
	}
	printf("\n Debug : Client 접속 : IP 주소 : %s \n", inet_ntoa(clientaddr.sin_addr));
	int i = 0;
	
	for (i = 0; i < g_UserF; i++)
	{
		if (Queue[i].sock == INVALID_SOCKET)
		{
			 dP = &Queue[i];
			 break;
		}
	}

	if (i == g_UserF)
	{
		dP = &Queue[g_UserCount];
		g_UserF = g_UserCount;
	}

	if (dP == NULL)
		return;

	dP->sock = client_sock;
	dP->bSendFlag = true;
	dP->SendQ.Initalize(10000);
	dP->RecvQ.Initalize(10000);

	g_UserCount++;

	int err = WSAAsyncSelect(client_sock, hMDlg, WM_NETWORK, FD_READ | FD_WRITE | FD_CLOSE);
	if (err == SOCKET_ERROR)
	{
		RemoveSocket(client_sock);
		return;
	}
}

// 해당 큐의 RecvQ 버퍼에 넣는다. -> 그 해당 큐의 RecvQ를 전체 클라이언트에 전송한다. -> 해당 큐를 초기화한다.
void RecvClientEvent(SOCKET sock)
{
	// 해당 소켓의 큐를 찾는다.
	int i = 0;

	for (i = 0; i <= g_UserF; i++)
	{
		if (sock == Queue[i].sock)
			break;
	}

	if (i == g_UserF + 1)
	{
		printf("Debug : 해당 소켓 없음 \n"); // 이럴리는 없지만..
		return;
	}

	st_USER_QUEUE *dQueue = &Queue[i];
	WSABUF wsabuf[2];
	int bufcount = 1;

	wsabuf[0].len = dQueue->RecvQ.GetNotBrokenPutSize();
	wsabuf[0].buf = dQueue->RecvQ.GetWriteBufferPtr();

	if (dQueue->RecvQ.GetNotBrokenPutSize() < dQueue->RecvQ.GetFreeSize())
	{
		bufcount++;

		wsabuf[1].len = dQueue->RecvQ.GetFreeSize() - wsabuf[0].len;
		wsabuf[1].buf = dQueue->RecvQ.GetBufferPtr();
	}

	DWORD RecvSize = 0;
	DWORD Flag = 0;

	int ret = WSARecv(sock, wsabuf, bufcount, &RecvSize, &Flag, NULL, NULL);
	if (ret == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			printf("WSARecv 에러 !! 에러 코드 : %d 해당 오류 소켓 끊음", WSAGetLastError());
			RemoveSocket(sock);
			return;
		}
	}

	dQueue->RecvQ.MoveWritePos(RecvSize);
	/*
	// 임시용 패킷 체크
	char recvTest[10000], recv1Test[10000];
	if (bufcount == 1)
	{
		memcpy(recvTest, wsabuf[0].buf, RecvSize);

		dQueue->RecvQ.Peek(recv1Test, RecvSize);
		if (memcmp(recvTest, recv1Test, RecvSize) != 0)
		{
			printf("Peek Error \n Peek Error \n Peek Error \n Peek Error \n Peek Error \n");
		}
	}
	if (bufcount == 2)
	{
		memcpy(recvTest, wsabuf[0].buf, wsabuf[0].len);
		memcpy(recvTest + wsabuf[0].len, wsabuf[1].buf, wsabuf[1].len);
		dQueue->RecvQ.Peek(recv1Test, RecvSize);

		if (memcmp(recvTest, recv1Test, RecvSize) != 0)
		{
			printf("Peek Error \n Peek Error \n Peek Error \n Peek Error \n Peek Error \n");
		}
	}
	*/
	PacketProc(&dQueue->RecvQ);
	
}

void PacketProc(CRingBuffer *pRecv)
{
	if (pRecv != NULL)
	{
		unsigned short len;
		st_PACKET Packet;
		static int befX = 0;
		static int befY = 0;

		while (1)
		{
			if (pRecv->GetUseSize() < 2)
				break;

			if (pRecv->Peek((char *)&len, 2) == 0)
				break;

			if (pRecv->GetUseSize() < len + 2)
				break;

			pRecv->Get((char *)&Packet, len + 2);
			
			// 디버그용 소스
			if (befX != 0 && befY != 0)
			{
				if (befX != Packet.iStartX && befY != Packet.iStartY)
				{
					st_PACKET T;
					T.len = 16;
					T.iStartX = befX;
					T.iStartY = befY;
					T.iEndX = Packet.iStartX;
					T.iEndY = Packet.iStartY;
					//SendDraw(&T);

					printf("Debug RecvError(이음새) : StartX = %d StartY = %d, EndX = %d EndY = %d \n", T.iStartX,
						T.iStartY, T.iEndX, T.iEndY);
				}
			}

			printf("Debug Recved : Len = %d StartX = %d StartY = %d, EndX = %d EndY = %d \n", Packet.len, Packet.iStartX,
				Packet.iStartY, Packet.iEndX, Packet.iEndY);

			befX = Packet.iEndX;
			befY = Packet.iEndY;
			////////////////////

			pRecv->RemoveData(len + 2);
			SendDraw(&Packet);
		}
	}
}
// 모든 활성된 소켓 버퍼에 패킷을 넣는다.
void SendDraw(st_PACKET *Packet)
{
	for (int i = 0; i <= g_UserF; i++)
	{
		if (Queue[i].sock != INVALID_SOCKET)
		{
			Queue[i].SendQ.Put((char *)Packet, 18);
		}
	}

	SendAll();
}

void SendClientEvent(SOCKET sock)
{
	// 해당 소켓의 SendFlag가 false라면 true로
	for (int i = 0; i < g_UserF; i++)
	{
		if (Queue[i].sock == sock)
		{
			if (Queue[i].bSendFlag == false)
				Queue[i].bSendFlag = true;

		}
	}
	SendAll();
}

void SendAll()
{
	for (int i = 0; i <= g_UserF; i++)
	{
		st_USER_QUEUE *pQueue = &Queue[i];

		if (pQueue->bSendFlag == true)
		{
			// 해당 큐에 데이터가 있다면 전송
			if (pQueue->SendQ.GetUseSize() > 0)
			{
				WSABUF wsabuf[2];
				int bufcount = 1;

				wsabuf[0].len = pQueue->SendQ.GetNotBrokenGetSize();
				wsabuf[0].buf = pQueue->SendQ.GetReadBufferPtr();

				if (pQueue->SendQ.GetUseSize() > wsabuf[0].len)
				{
					bufcount++;
					wsabuf[1].len = pQueue->SendQ.GetUseSize() - wsabuf[0].len;
					wsabuf[1].buf = pQueue->SendQ.GetBufferPtr();
				}

				DWORD SendSize = 0;
				DWORD Flag = 0;

				int err = WSASend(pQueue->sock, wsabuf, bufcount, &SendSize, Flag, NULL, NULL);
				if (err == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						pQueue->bSendFlag = false;
						continue;
					}
					else
					{
						printf("WSASend 에러 !! 에러 코드 : %d 해당 오류 소켓 끊음", WSAGetLastError());
						pQueue->bSendFlag = false;
						RemoveSocket(pQueue->sock);
						continue;
					}
				}

				pQueue->SendQ.ClearBuffer();
			}
		}
	}
}