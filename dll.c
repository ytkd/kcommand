#define STRICT
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400
#include <windows.h>

#include "msg.h"

static HHOOK vHook = 0;
static HWND AppWnd = 0;
static const int stroke[] = {VK_UP,VK_UP,VK_DOWN,VK_DOWN,VK_LEFT,VK_RIGHT,VK_LEFT,VK_RIGHT,'B','A', 0};
static int index = 0;
static DWORD pre_pos = 0;
static int vPause = 0;

static LRESULT CALLBACK MsgProc(int nCode, WPARAM wParam, LPARAM lParam);
int __stdcall InstallHook(HINSTANCE h, HWND hwnd);
int __stdcall UninstallHook(void);

BOOL __stdcall DllEntryPoint(HINSTANCE hInst, DWORD dwReason, void *p)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hInst);
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

int __stdcall InstallHook(HINSTANCE h, HWND hwnd)
{
	if (vHook == NULL) {
		AppWnd = hwnd;
		vHook = SetWindowsHookEx(WH_GETMESSAGE, MsgProc, h, 0);
	}
	return (int)vHook;
}

int __stdcall UninstallHook(void)
{
	index = 0;
	pre_pos = 0;

	if (vHook) {
		UnhookWindowsHookEx(vHook);
		vHook = 0;
	}
	return 0;
}


int __stdcall pause(int apause)
{
	vPause = apause;

	return 0;
}


static LRESULT CALLBACK MsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *pmsg;

	if (nCode < 0) {
		return CallNextHookEx(vHook, nCode, wParam, lParam);
	}

	pmsg = (MSG*)lParam;
	switch (pmsg->message) {
	case WM_KEYDOWN:
		do {
			if (pmsg->lParam & 0x40000000) {
				break;/* still hold */
			}
			if (stroke[index] == 0) {
				/* already power up */
				break;
			}
			if (pmsg->wParam == stroke[index]) {
				++index;
			}else{
				index = 0;
			}
			if (stroke[index] == 0) {
				POINT pt;
				GetCursorPos(&pt);
				pre_pos = (pt.y & 0xFFFF << 16) | (pt.x & 0xFFFF);
				/* power up */
				PostMessage(AppWnd, POWERUP, 0, 0);
			}
		} while (0);
		break;

	case WM_MOUSEMOVE:
	case WM_NCMOUSEMOVE:
		if (stroke[index] == 0 && pre_pos != pmsg->lParam) {
			/* already power up */
			pre_pos = pmsg->lParam;
			PostMessage(AppWnd, MOVEOPTION, 0, 0);
		}
		break;

	case WM_LBUTTONDOWN:
		if (!vPause) {
			PostMessage(AppWnd, LASER, 0, 0);
		}
		break;
	}
	return CallNextHookEx(vHook, nCode, wParam, lParam);
}

