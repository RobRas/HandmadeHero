#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer {
	// NOTE(Robert): Pixels are alwoys 32-bits wide, Memory Order BB GG RR XX
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
};

struct win32_window_dimensions {
	int Width;
	int Height;
};

// TODO(Robert): this is a global for now.
global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;

// Note(Robert): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
	return(0);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// Note(Robert): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
	return(0);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void Win32LoadXInput(void) {
	HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
	if (XInputLibrary) {
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		if (!XInputGetState) { XInputGetState = XInputGetStateStub; }
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
		if (!XInputSetState) { XInputSetState = XInputSetStateStub; }
	}
}

internal win32_window_dimensions Win32GetWindowDimensions(HWND Window) {
	win32_window_dimensions Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	
	return(Result);
}

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset) {
	// TODO(Robert): Let's see what the optimizer does
	uint8 *Row = (uint8 *)Buffer->Memory;
	for (int Y = 0; Y < Buffer->Height; Y++) {
		uint32 *Pixel = (uint32 *)Row;
		for (int X = 0; X < Buffer->Width; X++) {
			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
	}
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height) {
	// TODO(Robert): Bulletproof this.
	// Maybe don't free first, free after, then free first if that fails.

	if (Buffer->Memory) {
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	int BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;

	// NOTE(Robert): When the biHeight field is negative, this is the clue to Windows to treat
	// this bitmap as top-down, not bottom-up, meaning that the first three bytes of the image
	// are the color for the top left pixel in the bitmap, not the bottom left!
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	
	// TODO(Robert): Probably clear this to black.

	Buffer->Pitch = Buffer->Width*BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext,
										 int WindowWidth, int WindowHeight) {
	// TODO(Robert): Aspect ration correction
	// TODO(Robert): Play with stretch modes
	StretchDIBits(DeviceContext,
				  /*
				  X, Y, Width, Height
				  X, Y, Width, Height
				  */
				  0, 0, WindowWidth, WindowHeight,
				  0, 0, Buffer->Width, Buffer->Height,
				  Buffer->Memory,
				  &Buffer->Info,
				  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK MainWindowCallback(HWND Window,
									UINT Message,
									WPARAM WParam,
									LPARAM LParam) {
	LRESULT Result = 0;

	switch (Message) {
		case WM_CLOSE:
		{
			// TODO(Robert): Handle this with a message to the user?
			GlobalRunning = false;
		} break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP");
		} break;
		case WM_DESTROY:
		{
			// TODO(Robert): Handle this as an error - recreate window?
			GlobalRunning = false;
		} break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 VKCode = WParam;

			#define KeyMessageWasDownBit (1 << 30)
			#define KeyMessageIsDownBit (1 << 31)

			bool WasDown = ((LParam & KeyMessageWasDownBit) != 0);
			bool IsDown = ((LParam & KeyMessageIsDownBit) == 0);
			if (WasDown != IsDown) {
				if(VKCode == 'W') {
					OutputDebugString("W\n");
				} else if (VKCode == 'A') {
					OutputDebugString("A\n");
				} else if (VKCode == 'S') {
					OutputDebugString("S\n");
				} else if (VKCode == 'D') {
					OutputDebugString("D\n");
				} else if (VKCode == 'Q') {
					OutputDebugString("Q\n");
				} else if (VKCode == 'E') {
					OutputDebugString("E\n");
				} else if (VKCode == VK_UP) {
					OutputDebugString("Up\n");
				} else if (VKCode == VK_DOWN) {
					OutputDebugString("Down\n");
				} else if (VKCode == VK_LEFT) {
					OutputDebugString("Left\n");
				} else if (VKCode == VK_RIGHT) {
					OutputDebugString("Right\n");
				} else if (VKCode == VK_ESCAPE) {
					OutputDebugString("ESCAPE: ");
					if (IsDown) {
						OutputDebugString("IsDown ");
					}
					if (WasDown) {
						OutputDebugString("WasDown ");
					}
					OutputDebugString("\n");
				} else if (VKCode == VK_SPACE) {
					OutputDebugString("Space\n");
				}
			}
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);
			Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimensions.Width, Dimensions.Height);
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
	Win32LoadXInput();

	WNDCLASS WindowClass = {};

	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon = ;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&WindowClass)) {
		HWND Window = CreateWindowEx(
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
		if (Window) {
			// NOTE(Robert): Since we specified CS_OWNDC, we can just got one device context and
			// use it forever because we are not sharing it with anyone.
			HDC DeviceContext = GetDC(Window);

			int XOffset = 0;
			int YOffset = 0;

			GlobalRunning = true;
			while (GlobalRunning) {
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
					if (Message.message == WM_QUIT) {
						GlobalRunning = false;
					}

					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}

				// TODO(Robert): Should we poll this more frequently?
				for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++) {
					XINPUT_STATE ControllerState;
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
						// NOTE(Robert): This controller is plugged in
						// TODO(Robert): See if ControllerState.dwPacketNumber increments too rapidly
						XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
						bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
						bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
						bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
						bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
						bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;

						XOffset = StickX >> 12;
						YOffset = StickY >> 12;
					} else {
						// NOTE(Robert): This controller is not available
					}
				}

				XINPUT_VIBRATION Vibration;
				Vibration.wLeftMotorSpeed = 60000;
				Vibration.wRightMotorSpeed = 60000;
				XInputSetState(0, &Vibration);

				RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

				win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);
				Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimensions.Width, Dimensions.Height);

				++XOffset;
			}
		} else {
			// TODO(Robert): Logging
		}
	} else {
		// TODO(Robert): Logging
	}

	return(0);
}
