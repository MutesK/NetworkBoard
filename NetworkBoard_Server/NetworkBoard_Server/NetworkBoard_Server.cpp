// NetworkBoard_Server.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "NetworkBoard_Server.h"


#define MAX_LOADSTRING 100

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.
HWND hMDlg;
HWND g_hWnd;
HWND list;

int g_UserCount = 0;
int g_UserF = 0;
st_USER_QUEUE Queue[USER];
bool bSendFlag = false;

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK	DialogProc(HWND, UINT, WPARAM, LPARAM);

// ��Ʈ��ũ �Լ�
void RemoveSocket(SOCKET sock);
void AddClientSocket(SOCKET sock);
void RecvClientEvent(SOCKET sock);
void SendDraw(st_PACKET *Packet);
// WSAWOULDBLOCK ���� ���� �Լ�
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

    // TODO: ���⿡ �ڵ带 �Է��մϴ�.
	if (AllocConsole()) {
		freopen("CONIN$", "rb", stdin);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);
	}

    // ���� ���ڿ��� �ʱ�ȭ�մϴ�.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NETWORKBOARD_SERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);



    // ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	int err;
	// ���� �ʱ�ȭ
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

	printf("���� �����\n");
    MSG msg;

    // �⺻ �޽��� �����Դϴ�.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
    }

	WSACleanup();
    return (int) msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
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
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

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
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����:  �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND  - ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT    - �� â�� �׸��ϴ�.
//  WM_DESTROY  - ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // �޴� ������ ���� �м��մϴ�.
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
			// �ش� ���� ����
			printf("Error Socket !!");
			RemoveSocket(wParam);
			return 0;
		}

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			// ���� �߰�
			AddClientSocket(wParam);
			break;
		case FD_READ:
			RecvClientEvent(wParam); // �ش� ����
			break;
		case FD_WRITE:
			SendClientEvent(wParam);
			break;
		case FD_CLOSE:
			// �ش� ���� ����
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
			printf("\n Debug : Client ���� ���� : IP �ּ� : %s \n", inet_ntoa(clientaddr.sin_addr));
			SendMessage(list, LB_ADDSTRING, 0, (LPARAM)L"Client ���� ����");
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
	printf("\n Debug : Client ���� : IP �ּ� : %s \n", inet_ntoa(clientaddr.sin_addr));
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

// �ش� ť�� RecvQ ���ۿ� �ִ´�. -> �� �ش� ť�� RecvQ�� ��ü Ŭ���̾�Ʈ�� �����Ѵ�. -> �ش� ť�� �ʱ�ȭ�Ѵ�.
void RecvClientEvent(SOCKET sock)
{
	// �ش� ������ ť�� ã�´�.
	int i = 0;

	for (i = 0; i <= g_UserF; i++)
	{
		if (sock == Queue[i].sock)
			break;
	}

	if (i == g_UserF + 1)
	{
		printf("Debug : �ش� ���� ���� \n"); // �̷����� ������..
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
			printf("WSARecv ���� !! ���� �ڵ� : %d �ش� ���� ���� ����", WSAGetLastError());
			RemoveSocket(sock);
			return;
		}
	}

	dQueue->RecvQ.MoveWritePos(RecvSize);
	/*
	// �ӽÿ� ��Ŷ üũ
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
			
			// ����׿� �ҽ�
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

					printf("Debug RecvError(������) : StartX = %d StartY = %d, EndX = %d EndY = %d \n", T.iStartX,
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
// ��� Ȱ���� ���� ���ۿ� ��Ŷ�� �ִ´�.
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
	// �ش� ������ SendFlag�� false��� true��
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
			// �ش� ť�� �����Ͱ� �ִٸ� ����
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
						printf("WSASend ���� !! ���� �ڵ� : %d �ش� ���� ���� ����", WSAGetLastError());
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