// NetworkBoard.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//
#include "stdafx.h"
#include "NetworkBoard.h"
#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND g_hWnd;

// 응용 프로그램 전역변수들
WCHAR g_szIP[16];  // IP 주소 변수

// 그리기 전용 전역변수
bool isLDown = false;
HBITMAP g_hMemDC_Bitmap;
HDC g_hDC;
HBITMAP g_hMemDC_OldBitmap;
RECT rect;

// 네트워크 전용 전역변수
st_Packet Packet;
bool bConnect = false;
bool bSendFlag = false;
SOCKET sock;
CRingBuffer RecvQ(10000);
CRingBuffer SendQ(10000);


// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR	CALLBACK	DialogProc(HWND, UINT, WPARAM, LPARAM);

// 네트워크 기본 설정
bool NetworkInit();
void SendDraw(int sX, int sY, int eX, int eY);
void SendEvent();
void RecvEvent();
void PacketProc();

void Draw();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);



	// 다이얼로그 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ADDR), NULL, DialogProc);

    
    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NETWORKBOARD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);



    // 응용 프로그램 초기화를 수행합니다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

	// 기본 메시지 루프입니다.
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	WSACleanup();
	closesocket(sock);
    return (int) msg.wParam;
}

bool NetworkInit()
{
	int err;
	WSADATA wsa;
	SOCKADDR_IN serveraddr;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 0;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		return 0;
	}

	WSAAsyncSelect(sock, g_hWnd, WM_NETWORK, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	InetPton(AF_INET, g_szIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(PORT);

	err = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (err == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			return 0;
		}
	}

	return 1;

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NETWORKBOARD));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;
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

   g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);

   if (!g_hWnd)
   {
      return FALSE;
   }

   NetworkInit();


	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);
   

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
	static int x = 0, y = 0;

    switch (message)
    {
	case WM_CREATE:
	{
		GetClientRect(hWnd, &rect);

		HDC hdc = GetDC(hWnd);

		g_hDC = CreateCompatibleDC(hdc);
		g_hMemDC_Bitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		g_hMemDC_OldBitmap = (HBITMAP)SelectObject(g_hDC, g_hMemDC_Bitmap);

		ReleaseDC(hWnd, hdc);

		// 패턴 블릿 -> 새로 만든 HDC 초기화
		PatBlt(g_hDC, 0, 0, rect.right, rect.bottom, WHITENESS);
	}
		break;
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		isLDown = true;
		break;
	case WM_LBUTTONUP:
		isLDown = false;
		break;
	case WM_MOUSEMOVE:
		if (isLDown)
		{
			int x2 = LOWORD(lParam);
			int y2 = HIWORD(lParam);

			if (x != x2 && y != y2)
			{
				SendDraw(x, y, x2, y2);
				// 위치전송
				x = x2;
				y = y2;
			}
		}
		break;
	case WM_NETWORK:
		switch (lParam)
		{
		case FD_CONNECT:
				bConnect = true;
			break;
		case FD_CLOSE:
			if (bConnect)
			{
				bConnect = false;
				closesocket(sock);
			}
			break;
		case FD_READ:
			if (bConnect)
				RecvEvent();
			break;
		case FD_WRITE:
			if (bConnect)
			{
				bSendFlag = true;
				SendEvent();
			}
			break;
		}
		break;
    case WM_PAINT:
        {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.

			RECT rect;
			GetClientRect(hWnd, &rect);

			BitBlt(hdc, 0, 0, rect.right, rect.bottom, g_hDC, 0, 0, SRCCOPY);
			ReleaseDC(hWnd, hdc);

			EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		SelectObject(g_hDC, g_hMemDC_OldBitmap);
		DeleteObject(g_hMemDC_Bitmap);
		DeleteDC(g_hDC);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR CALLBACK DialogProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hEditBox;

	switch (iMsg)
	{
	case WM_INITDIALOG:		// 다이얼로그 생성시 발생(초기화용)
		memset(g_szIP, 0, sizeof(WCHAR) * 16);
		hEditBox = GetDlgItem(hWnd, IDC_EDIT1);
		SetWindowText(hEditBox, L"127.0.0.1");
		return true;
	case WM_COMMAND:		// 다이얼로그 내부의 컨트롤 동작, 변경시 발생
		switch (wParam)
		{
		case IDOK:
			GetDlgItemText(hWnd, IDC_EDIT1, g_szIP, 16);
			EndDialog(hWnd, 99939);
		}
		return true;
	}

	return false;
}

void SendDraw(int sX, int sY, int eX, int eY)
{
	if (bConnect)
	{
		Packet.iStartX = sX;
		Packet.iStartY = sY;
		Packet.iEndX = eX;
		Packet.iEndY = eY;

		Packet.len = 16;

		// SendQ에 넣는다.
		SendQ.Put(((char*)&Packet), sizeof(Packet));
		SendEvent();

		memset(&Packet, 0, sizeof(Packet));
	}
}

void SendEvent()
{
	if (bSendFlag == false)
		return;

	if (SendQ.GetUseSize() <= 0)
		return;

	WSABUF wsabuf[2];
	int bufcount = 1;


	// 사용중인 사이즈 보다 쓸수있는 량이 적을때
	wsabuf[0].len = SendQ.GetNotBrokenGetSize();
	wsabuf[0].buf = SendQ.GetReadBufferPtr();
	if (SendQ.GetUseSize() > SendQ.GetNotBrokenGetSize())
	{
		bufcount++;
		wsabuf[1].len = SendQ.GetUseSize() - SendQ.GetNotBrokenGetSize();
		wsabuf[1].buf = SendQ.GetBufferPtr();
	}
	

	DWORD SendSize = 0;
	DWORD Flag = 0;

	
	int ret = WSASend(sock, wsabuf, bufcount, &SendSize, Flag, NULL, NULL);
	if (ret == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			bSendFlag = false;
			return;
		}
		else
		{
			MessageBox(g_hWnd, L"소켓 에러", L"Error", MB_OK);
			exit(1);
		}
	}
	
	
	SendQ.RemoveData(SendSize);
}


void RecvEvent()
{
	WSABUF wsabuf[2];
	int bufcount = 1;

	wsabuf[0].len = RecvQ.GetNotBrokenPutSize();
	wsabuf[0].buf = RecvQ.GetWriteBufferPtr();

	if (RecvQ.GetNotBrokenPutSize() < RecvQ.GetFreeSize())
	{
		// 아직 공간이 남아 있다면
		wsabuf[1].len = RecvQ.GetFreeSize() - wsabuf[0].len;
		wsabuf[1].buf = RecvQ.GetBufferPtr();
		bufcount++;
	}
	
	DWORD SendSize = 0;
	DWORD Flag = 0;

	int ret = WSARecv(sock, wsabuf, bufcount, &SendSize, &Flag, NULL, NULL);
	if (ret == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			WCHAR szTest[250];
			wsprintf(szTest, L"Error = %d \n", WSAGetLastError());
			OutputDebugString(szTest);

			MessageBox(g_hWnd, L"소켓 에러", L"Error", MB_OK);
			exit(1);
		}
	}
	

	RecvQ.MoveWritePos(SendSize);
	PacketProc();
}

void PacketProc()
{
	unsigned short len;

	while (1)
	{
		if (RecvQ.GetUseSize() < 2)
			break;

		if (RecvQ.Peek((char *)&len, 2) == 0)
			break;
		
		if (RecvQ.GetUseSize() < len + 2)
			break;

		RecvQ.Get(((char *)&Packet), len + 2);

		RecvQ.RemoveData(len + 2);
		Draw();
	}

}
void Draw()
{
	MoveToEx(g_hDC, Packet.iStartX, Packet.iStartY, NULL);
	LineTo(g_hDC, Packet.iEndX, Packet.iEndY);

	InvalidateRect(g_hWnd, NULL, FALSE);
	memset(&Packet, 0, sizeof(Packet));
}