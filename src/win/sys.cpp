#include "windows.h"
#include "winsock.h"
#include "main.h"
#include "utils.h"

#ifdef _DEBUG
int main(int argc, char* argv[]) {
	_CrtMemState _ms;	
	_CrtMemCheckpoint(&_ms);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#else	
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
int main(int argc, char* argv[]) {
#endif
	WSAData wData;
	WSAStartup(0x0101, &wData);

	Main::init();
	Main::deinit();

	WSACleanup();
#ifdef _DEBUG
	_CrtMemDumpAllObjectsSince(&_ms);
	system("pause");
#endif
	return 0;
}

HWND wHandle;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	Window *wnd = (Window*)GetWindowLong(hWnd, GWL_USERDATA);

	PAINTSTRUCT ps;
	RECT &r = ps.rcPaint;
	HDC DC, mDC;
	HBITMAP mBMP, oBMP;
	MINMAXINFO *mmInfo;

	switch (message) {
		case WM_CLOSE :
			PostQuitMessage(0);
			return 1;
		case WM_GETMINMAXINFO :
			mmInfo = (MINMAXINFO*)lParam;
			mmInfo->ptMinTrackSize.x = 320;
			mmInfo->ptMinTrackSize.y = 240;
			break;
		case WM_ACTIVATEAPP :
			if (short(wParam)) {
			//	resume
			} else {
			//	pause
			}
			break;
		case WM_ERASEBKGND :
			return 1;
		case WM_PAINT :
			DC   = BeginPaint(hWnd, &ps);
			mBMP = CreateCompatibleBitmap(DC, r.right - r.left, r.bottom - r.top);
			mDC  = CreateCompatibleDC(DC);
			oBMP = (HBITMAP)SelectObject(mDC, mBMP);
			SetWindowOrgEx(mDC, r.left, r.top, 0);		
		/*
			LDC := Wnd.Canvas.DC;
			Wnd.Canvas.DC := MDC;
			Event.ID := etDraw;
			Event.Draw.Canvas := Wnd.Canvas;
			Event.Draw.Rect   := Wnd.Canvas.PaintRect;
			Wnd.Perform(Event);
			Wnd.Canvas.DC := LDC;
		*/
			wnd->draw();

			BitBlt(DC, r.left, r.top, r.right - r.left, r.bottom - r.top, mDC, r.left, r.top, SRCCOPY);
			SelectObject(mDC, oBMP);
			DeleteObject(mBMP);
			DeleteDC(mDC);
	        EndPaint(hWnd, &ps);
			break;
		default:
			return DefWindowProcA(hWnd, message, wParam, lParam);
	}
	return 0;
}

Window::Window() : Control() {
	wHandle = CreateWindowEx(WS_EX_APPWINDOW, "STATIC", "Arcanum", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, 0, 0);
	SetWindowLong(wHandle, GWL_USERDATA, LONG(this));
	SetWindowLong(wHandle, GWL_WNDPROC, LONG(&WndProc));
}

Window::~Window() {
	//
}

void Window::msgLoop() {
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DestroyWindow(wHandle);
}

__int64 toFileTime(const SYSTEMTIME &time) {
	FILETIME ftime;
	LARGE_INTEGER sec;
	SystemTimeToFileTime(&time, &ftime);
	sec.HighPart = ftime.dwHighDateTime;
	sec.LowPart  = ftime.dwLowDateTime;
	return sec.QuadPart;
}

__int64 getUnixTime() {
	SYSTEMTIME start = {1970, 1, 3, 1, 0, 0, 0, 0};
	SYSTEMTIME now;	
	GetSystemTime(&now);
	return (toFileTime(now) - toFileTime(start)) / 10000000;
}