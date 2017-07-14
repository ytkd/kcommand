#define WindowClass "BASE"
#define OptionWindowClass "Option"
#define LaserWindowClass "Laser"
#define szAppName "window"


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OptProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern HINSTANCE vhInst;

