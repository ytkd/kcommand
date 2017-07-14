#define WIN32_LEAN_AND_MEAN
#define STRICT
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>

#include "base.h"
#include "res.h"
#include "msg.h"

#define OPTION_PERIOD 16
#define OPTION_BUFFER ((OPTION_PERIOD) * 5)
#define OPTION_CX 64
#define OPTION_CY 48
#define LASER_CY 4

#define NOPTIONS 4
#define NBRUSH   8


static NOTIFYICONDATA ni = {
	sizeof(NOTIFYICONDATA),
	NULL,
	1,
	NIF_MESSAGE|NIF_ICON|NIF_TIP,
	TRAYMESSAGE,
	NULL,
	"コナ○コマンド"
};

static HMENU hMenu;
static HANDLE dll;

static int install_hook(HWND hwnd);
static void uninstall_hook(void);
static void LaserPause(int apause);

static HWND hwndOpt[NOPTIONS];
static HWND hwndLaser[NOPTIONS];
static POINT trace[OPTION_BUFFER];
static HBRUSH hbrOptions[NBRUSH];
static HBRUSH hbrOptBack;
static int viBack;
static int vnDelta;
static int vnLaserCX;
static int vbLaser;
static int vnTracer;


int on_create(HWND hwnd)
{
	int i;
	int offset;
	static const int base_color_r = 240;
	static const int base_color_g = 192;
	static const int base_color_b =  80;
	static const int lite_color_r = 255;
	static const int lite_color_g = 240;
	static const int lite_color_b = 160;
	static const int dark_color_r = 192;
	static const int dark_color_g =  80;
	static const int dark_color_b =   0;
	int r, g, b;
	int ldelta_r,ldelta_g,ldelta_b;
	int ddelta_r,ddelta_g,ddelta_b;

	if (!install_hook(hwnd)) {
		return -1;/* to break to create window */
	}

	ni.hWnd = hwnd;
	ni.hIcon = (HICON)LoadIcon(vhInst, MAKEINTRESOURCE(IDI_ICON));
	Shell_NotifyIcon(NIM_ADD, &ni);
	hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, IDM_QUIT, "終了");

	offset = 1;
	ldelta_r = lite_color_r - base_color_r;
	ldelta_g = lite_color_g - base_color_g;
	ldelta_b = lite_color_b - base_color_b;
	ddelta_r = base_color_r - dark_color_r;
	ddelta_g = base_color_g - dark_color_g;
	ddelta_b = base_color_b - dark_color_b;
	for (i = 0; i < NBRUSH / 2; i++) {
		r = base_color_r + (ldelta_r * offset) / 8;
		g = base_color_g + (ldelta_g * offset) / 8;
		b = base_color_b + (ldelta_b * offset) / 8;
		hbrOptions[NBRUSH / 2 - 1 - i] = CreateSolidBrush(RGB(r, g, b));

		r = base_color_r - (ddelta_r * offset) / 8;
		g = base_color_g - (ddelta_g * offset) / 8;
		b = base_color_b - (ddelta_b * offset) / 8;
		hbrOptions[NBRUSH / 2 + i] = CreateSolidBrush(RGB(r, g, b));
		offset *= 2;
	}
	return 0;
}


void on_destroy(HWND hwnd)
{
	int i;

	Shell_NotifyIcon(NIM_DELETE, &ni);
	if (dll) {
		uninstall_hook();
		FreeLibrary(dll);
		dll = NULL;
	}
	DestroyMenu(hMenu);

	if (hwndOpt[0]) {
		vbLaser = 0;
		KillTimer(hwnd, 1);
		SetClassLong(hwndOpt[0], GCL_HBRBACKGROUND, (LONG)hbrOptBack);
		for (i = 0; i < NOPTIONS; ++i) {
			DestroyWindow(hwndOpt[i]);
			hwndOpt[i] = NULL;
			DestroyWindow(hwndLaser[i]);
			hwndLaser[i] = NULL;
		}
		for (i = 0; i < NBRUSH; i++) {
			DeleteObject(hbrOptions[i]);
		}
	}
	PostQuitMessage(0);
}


void on_command(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
	case IDM_QUIT:
		DestroyWindow(hwnd);
		break;
	}
}

void on_tray_message(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	POINT pt;

	if (wParam == 1 && lParam == WM_LBUTTONDOWN) {
		SetForegroundWindow(hwnd);
		GetCursorPos(&pt);
		LaserPause(1);
		TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hwnd, NULL);
		LaserPause(0);
	}
}

void on_paint(HWND hwnd)
{
	HDC dc;
	PAINTSTRUCT ps;

	dc = BeginPaint(hwnd, &ps);
	EndPaint(hwnd, &ps);
}

void do_laser()
{
	int i;
	int n;
	
	if (vbLaser) {
		return;
	}

	vbLaser = 1;
	vnLaserCX = OPTION_CX;
	n = vnTracer;
	for (i = 0; i < NOPTIONS; i++) {
		if (n < OPTION_PERIOD) {
			n += OPTION_BUFFER;
		}
		n -= OPTION_PERIOD;
		SetWindowPos(hwndLaser[i], NULL,
			     trace[n].x + (OPTION_CX / 2),
			     trace[n].y - (LASER_CY / 2),
			     vnLaserCX, LASER_CY,
			     SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER|SWP_SHOWWINDOW);
	}
}

void on_timer(HWND hwnd, WPARAM wParam)
{
	int i;

	if (!hwndOpt[0]) {
		return;
	}

	switch (wParam) {
	case 1:
		SetClassLong(hwndOpt[0], GCL_HBRBACKGROUND, (LONG)hbrOptions[viBack]);
		viBack += vnDelta;
		if (NBRUSH <= viBack) {
			viBack = NBRUSH - 2;
			vnDelta = -1;
		}
		if (viBack < 0) {
			viBack = 1;
			vnDelta = 1;
		}
		for (i = 0; i < NOPTIONS; i++) {
			InvalidateRect(hwndOpt[i], NULL, TRUE);
		}

		if (vbLaser) {
			vnLaserCX += 128;
			if (1024 <= vnLaserCX) {
				vbLaser = 0;
				vnLaserCX = 0;
				for (i = 0; i < NOPTIONS; i++) {
					SetWindowPos(hwndLaser[i], NULL, 0, 0, 0, 0,
						     SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_HIDEWINDOW);
				}
			} else {
				for (i = 0; i < NOPTIONS; i++) {
					SetWindowPos(hwndLaser[i], NULL,
						     0, 0,
						     vnLaserCX, LASER_CY,
						     SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW);
				}
			}
		}

		break;
	}
}


void CreateOptions(HWND hwnd)
{
	POINT pt;
	int i;
	HRGN rgn;
	const static RECT rc = {0, 0, OPTION_CX, OPTION_CY};

	if (hwndOpt[0]) {
		return;
	}

	viBack = 0;
	vnDelta = 1;
	hbrOptBack = (HBRUSH)SetClassLong(hwndOpt[0], GCL_HBRBACKGROUND, (LONG)hbrOptions[viBack]);
	GetCursorPos(&pt);
	for (i = 0; i < OPTION_BUFFER; ++i) {
		trace[i] = pt;
	}

	for (i = 0; i < NOPTIONS; ++i) {
		hwndOpt[i] = CreateWindowEx(
			WS_EX_TOPMOST,
			OptionWindowClass, "",
			WS_POPUP | WS_DISABLED,
			pt.x - (OPTION_CX / 2), pt.y - (OPTION_CY / 2),
			OPTION_CX, OPTION_CY,
			hwnd,
			NULL,
			vhInst,
			NULL);
		if (hwndOpt[i]) {
			rgn = CreateEllipticRgnIndirect(&rc);
			SetWindowRgn(hwndOpt[i], rgn, TRUE);
		}

		hwndLaser[i] = CreateWindowEx(
			WS_EX_TOPMOST,
			LaserWindowClass, "",
			WS_POPUP | WS_DISABLED,
			pt.x - (OPTION_CX / 2), pt.y - (OPTION_CY / 2),
			OPTION_CX, LASER_CY,
			hwnd,
			NULL,
			vhInst,
			NULL);
	}
	for (i = 0; i < NOPTIONS; ++i) {
		SetWindowPos(hwndOpt[i], NULL, 0, 0, 0, 0,
			     SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
	}
	SetTimer(hwnd, 1, 100, NULL);
}

void MoveOptions(HWND hwnd)
{
	int i;
	int opt;
	POINT pt;

	GetCursorPos(&pt);
	if (pt.x == trace[vnTracer].x && pt.y == trace[vnTracer].y) {
		return;
	}

	++vnTracer;
	if (OPTION_BUFFER <= vnTracer) {
		vnTracer = 0;
	}

	trace[vnTracer] = pt;

	i = vnTracer;
	for (opt = 0; opt < NOPTIONS && hwndOpt[opt]; ++opt) {
		if (i < OPTION_PERIOD) {
			i += OPTION_BUFFER;
		}
		i -= OPTION_PERIOD;
		SetWindowPos(hwndOpt[opt], NULL,
			     trace[i].x - (OPTION_CX / 2),
			     trace[i].y - (OPTION_CY / 2),
			     0, 0,
			     SWP_NOACTIVATE | SWP_NOSIZE);
		if (vbLaser) {
			SetWindowPos(hwndLaser[opt], NULL,
				     trace[i].x + (OPTION_CX / 2),
				     trace[i].y - (LASER_CY / 2),
				     vnLaserCX, LASER_CY,
				     SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER|SWP_SHOWWINDOW);
		}
	}
}


/* install_hook system hook */
static int install_hook(HWND hwnd)
{
	int (__stdcall *install_hook)(HINSTANCE, HWND);

	dll = LoadLibrary("kchook.dll");
	if (!dll) {
		MessageBox(NULL, "can't load DLL.", "Error", 0);
		return 0;
	}

	install_hook = GetProcAddress(dll, "InstallHook");
	if (!install_hook || !install_hook(dll, hwnd)) {
		MessageBox(NULL, "can't bind procedure.", "Error", 0);
		return 0;
	}

	return 1;
}

/* uninstall_hook system hook */
static void uninstall_hook(void)
{
	void (CALLBACK *uninst)(void);

	if (!dll) {
		return;
	}

	uninst = (void*)GetProcAddress(dll, "UninstallHook");
	if (uninst) {
		uninst();
	}
}


static void LaserPause(int apause)
{
	void (CALLBACK *pause)(int);

	if (!dll) {
		return;
	}

	pause = (void*)GetProcAddress(dll, "LaserPause");
	if (pause) {
		pause(apause);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {/*  WM_xxxx  */
	case WM_CREATE:
		return on_create(hwnd);
		break;
	case WM_DESTROY:
		on_destroy(hwnd);
		break;
	case WM_ACTIVATE:
		break;
	case WM_COMMAND:
		on_command(hwnd, wParam, lParam);
		break;
	case WM_PAINT:
		on_paint(hwnd);
		break;
	case WM_TIMER:
		on_timer(hwnd, wParam);
		break;
	case TRAYMESSAGE:
		on_tray_message(hwnd, wParam, lParam);
		break;
	case POWERUP:
		CreateOptions(hwnd);
		break;
	case MOVEOPTION:
		MoveOptions(hwnd);
		break;
	case LASER:
		do_laser();
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0L;
}

