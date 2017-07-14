#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <mmsystem.h>

#include "base.h"

HINSTANCE vhInst;

void Entry(void);

HWND create_app_window(void);


#ifndef RELEASE
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pszCmdLine, int nCmdShow)
{
	Entry();
	return 0;
}
#endif

int InitApplication(void);

void Entry(void)
{
	MSG msg;
	HWND hwnd;

	vhInst = GetModuleHandle(NULL);

	if (!InitApplication()) {
		msg.wParam = 1;
		goto EXIT;
	}
	hwnd = create_app_window();
	if (!hwnd) {
		msg.wParam = 1;
		goto EXIT;
	}

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


EXIT:
	ExitProcess(msg.wParam);
}

void FreeObject(HANDLE h)
{
	if (h) {
		DeleteObject(h);
	}
}


HWND create_app_window(void)
{
	HWND hwnd;

	hwnd = CreateWindowEx(
		0,
		WindowClass, szAppName,
		0,
		CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0,
		NULL, NULL, vhInst, NULL);

	return hwnd;
}


int InitApplication(void)
{
	WNDCLASS wc;

	/* for hidden window */
	wc.style         = CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_BYTEALIGNCLIENT;
	wc.lpfnWndProc   = (WNDPROC)WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = vhInst;
	wc.hIcon         = LoadIcon( NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = WindowClass;

	if (!RegisterClass(&wc)) {
		return 0;
	}

	/* for option window */
	wc.lpfnWndProc   = (WNDPROC)OptProc;
	wc.lpszClassName = OptionWindowClass;
	wc.hbrBackground = CreateSolidBrush(0x0050c0f0);

	RegisterClass(&wc);

	/* for option window */
	wc.lpfnWndProc   = (WNDPROC)OptProc;
	wc.lpszClassName = LaserWindowClass;
	wc.hbrBackground = CreateSolidBrush(RGB(160, 160, 255));

	return RegisterClass(&wc);

}

