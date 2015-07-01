#include <windows.h>

LRESULT CALLBACK MainWindowCallback(HWND Window,
									UINT Message,
									WPARAM WParam,
									LPARAM LParam) {
	LRESULT Result = 0;

	switch (Message) {
		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZE");
		} break;
		case WM_DESTROY:
		{
			OutputDebugStringA("WM_DESTROY");
		} break;
		case WM_CLOSE:
		{
			OutputDebugStringA("WM_CLOSE");
		} break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			static DWORD Operation = WHITENESS;
			PatBlt(DeviceContext, X, Y, Width, Height, Operation);
			if (Operation == WHITENESS) {
				Operation = BLACKNESS;
			} else {
				Operation = WHITENESS;
			}
			EndPaint(Window, &Paint);
		} break;

		default:
		{
			//OutputDebugStringA("default");
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return(Result);
}

int CALLBACK WinMain(HINSTANCE Instance,
					 HINSTANCE PrevInstance,
					 LPSTR CommandLine,
					 int ShowCode) {
	WNDCLASS WindowClass = {};

	//Check if HREDRAW/VREDROW/OWNDC still matter
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon = ;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&WindowClass)) {
		HWND WindowHandle = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0);
		if (WindowHandle) {
			MSG Message;
			for (;;) {
				BOOL MessageResult = GetMessage(&Message, 0, 0, 0);
				if (MessageResult > 0) {
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				} else {
					break;
				}
			}
		} else {

		}
	} else {
		//TODO: Logging
	}

	return(0);
}
