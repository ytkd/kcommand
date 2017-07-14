#define WIN32_LEAN_AND_MEAN
#define STRICT
#define _WIN32_WINNT 0x0400
#include <windows.h>

LRESULT CALLBACK OptProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
