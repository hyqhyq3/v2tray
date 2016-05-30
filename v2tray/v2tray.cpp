// v2tray.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "v2tray.h"
#include "v2ray.h"
#include <shellapi.h>
#include <stdio.h>
#include "WinHttp.h"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wininet.lib")

#define MAX_LOADSTRING 100
#define WM_ICON WM_USER + 1
#define MENU_RELOAD WM_USER + 2
#define MENU_EXIT WM_USER + 3
#define MENU_EDIT WM_USER + 4
#define MENU_PROXY WM_USER + 5
#define MENU_NOPROXY WM_USER + 6
#define MENU_SHOWCONSOLE WM_USER + 7

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HANDLE m_hStartEvent;

NOTIFYICONDATA nid{};
V2Ray v2ray;


// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CheckOneInstance();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


	if (!CheckOneInstance())
	{
		return 1;
	}


    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_V2RAYTRAY, szWindowClass, MAX_LOADSTRING);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_V2RAYTRAY));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

BOOL CheckOneInstance()
{
	m_hStartEvent = CreateEventW(NULL, TRUE, FALSE, L"EVENT_NAME_HERE");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(m_hStartEvent);
		m_hStartEvent = NULL;
		// already exist
		// send message from here to existing copy of the application
		return FALSE;
	}
	// the only instance, start in a usual way
	return TRUE;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   WNDCLASS wndClass{};
   wndClass.hInstance = hInstance;
   wndClass.lpfnWndProc = &WndProc;
   wndClass.lpszClassName = szWindowClass;
   
   RegisterClass(&wndClass);

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0, hInstance, 0);
   ShowWindow(hWnd, SW_HIDE);

   HICON hIco = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_V2RAYTRAY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
   nid.cbSize = sizeof(nid);
   nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE;
   nid.hIcon = hIco;
   nid.hWnd = hWnd;
   wcscpy_s(nid.szTip, _T("v2ray"));
   nid.uVersion = NOTIFYICON_VERSION_4;
   nid.uCallbackMessage = WM_ICON;
   Shell_NotifyIcon(NIM_ADD, &nid);

   v2ray.init(hWnd, "v2ray.ini");

   return TRUE;
}

void popMenu(HINSTANCE hInst, HWND hWnd)
{
	MENUITEMINFO separatorBtn = { 0 };
	separatorBtn.cbSize = sizeof(MENUITEMINFO);
	separatorBtn.fMask = MIIM_FTYPE;
	separatorBtn.fType = MFT_SEPARATOR;

	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, -1, MF_BYPOSITION, MENU_RELOAD, _T("重载"));
		InsertMenu(hMenu, -1, MF_BYPOSITION, MENU_EDIT, _T("编辑配置"));

		HMENU hProxyMenu = CreatePopupMenu();
		if (hProxyMenu)
		{
			WINHTTP_CURRENT_USER_IE_PROXY_CONFIG proxyInfo;
			const TCHAR* szProxy = _T("http://localhost:5000");
			WinHttpGetIEProxyConfigForCurrentUser(&proxyInfo);
			if (proxyInfo.lpszProxy == NULL)
			{
				InsertMenu(hProxyMenu, -1, MF_BYPOSITION | MF_CHECKED, MENU_NOPROXY, _T("不使用代理"));
				InsertMenu(hProxyMenu, -1, MF_BYPOSITION, MENU_PROXY, szProxy);
			}
			else
			{
				InsertMenu(hProxyMenu, -1, MF_BYPOSITION, MENU_NOPROXY, _T("不使用代理"));
				if (_tcscmp(proxyInfo.lpszProxy, szProxy) == 0)
				{
					InsertMenu(hProxyMenu, -1, MF_BYPOSITION|MF_CHECKED, MENU_PROXY, szProxy);
				}
				else
				{
					InsertMenu(hProxyMenu, -1, MF_BYPOSITION , MENU_PROXY, szProxy);
				}
			}
		}
		InsertMenu(hMenu, -1, MF_STRING | MF_POPUP, (UINT)hProxyMenu, _T("设置代理"));
		InsertMenu(hMenu, -1, MF_BYPOSITION, MENU_SHOWCONSOLE, _T("显示控制台"));
		InsertMenu(hMenu, -1, MF_BYPOSITION, MENU_EXIT, _T("退出"));

		POINT pt;
		GetCursorPos(&pt);
		SetForegroundWindow(hWnd);
		TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
		PostMessage(hWnd, WM_NULL, 0, 0);

		DestroyMenu(hMenu);
		if (hProxyMenu)
		{
			DestroyMenu(hProxyMenu);
		}
	}
}

void showConsole()
{
	if (AllocConsole())
	{
		freopen("CONOUT$", "w", stdout);
	}
	else
	{
		HWND hWnd = GetConsoleWindow();
		ShowWindow(hWnd, IsWindowVisible(hWnd) ? SW_HIDE : SW_SHOW);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
				break;
			case MENU_RELOAD:
			{
				v2ray.restart();
				break;
			}
			case MENU_EDIT:
			{
				v2ray.editConfig();
				break;
			}
			case MENU_PROXY:
			{
				v2ray.setProxy();
				break;
			}
			case MENU_SHOWCONSOLE:
			{
				showConsole();

				break;
			}
			case MENU_NOPROXY:
			{
				v2ray.setNoProxy();
				break;
			}
			case MENU_EXIT:
			{
				DestroyWindow(hWnd);
				break;
			}
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_ICON:
	{
		int nmId = LOWORD(lParam);
		// 分析菜单选择: 
		switch (nmId)
		{
		case WM_CONTEXTMENU:
			DestroyWindow(hWnd);
			break;
		case WM_RBUTTONUP:
			popMenu(hInst, hWnd);
			break;
		case WM_LBUTTONUP:
			showConsole();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
	}
    return 0;
}