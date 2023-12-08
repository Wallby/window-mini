#include "window_mini.h"

#if defined(_WIN32)
//...
#elif defined(__linux__)
//...
#else
#error "unsupported os"
#endif

// NOTE: assumes..
//       .. numElementsToAppend > 0
static void add_or_append_num_elements(int elementSize, int* numElements, void** elements, int numElementsToAppendOrAdd)
{
	void* a = *elements;
	//*elements = (void*)new char[elementSize * ((*numElements) + numElementsToAppendOrAdd)];
	*elements = malloc(elementSize * ((*numElements) + numElementsToAppendOrAdd));	
	if(*numElements > 0)
	{
		memcpy(*elements, a, elementSize * (*numElements));
		//delete a;
		free(a);		
	}
	*numElements += numElementsToAppendOrAdd;
}
//#define add_or_append_num_elements2(numElements, elements, numElementsToAppendOrAdd) add_or_append_num_elements(sizeof **elements, numElements, (void**)elements, numElementsToAppendOrAdd)
#define add_or_append_one_element(elementSize, numElements, elements) add_or_append_num_elements(elementSize, numElements, elements, 1)
#define add_or_append_one_element2(numElements, elements) add_or_append_one_element(sizeof **elements, numElements, (void**)elements)

// NOTE: assumes..
//       .. *numElements >= lastNumElementsToRemove
//       .. lastNumElementsToRemove > 0
static void remove_last_num_elements(int elementSize, int* numElements, void** elements, int lastNumElementsToRemove)
{
	if(*numElements == lastNumElementsToRemove)
	{
		//delete *elements;
		free(*elements);
	}
	else
	{
		void* a = *elements;
		//*elements = (void*)new char[elementSize * ((*numElements) - lastNumElementsToRemove)];
		*elements = malloc(elementSize * ((*numElements) - lastNumElementsToRemove));
		memcpy(*elements, a, elementSize * ((*numElements) - lastNumElementsToRemove));
		//delete a;
		free(a);
	}
	*numElements -= lastNumElementsToRemove;
}
#define remove_last_num_elements2(numElements, elements, lastNumElementsToRemove) remove_last_num_elements(sizeof **elements, numElements, (void**)elements, lastNumElementsToRemove)

#if defined(_WIN32)
static void getlasterror_to_string(int* a, char* b)
{
	DWORD c = GetLastError();

	char* d;
	// NOTE: FORMAT_MESSAGE_MAX_WIDTH_MASK such that "[FormatMessageA]..
	//       .. ignores regular line breaks in the message definition..
	//       .. text"
	//       ^
	//       https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagea
	// NOTE: currently assuming US English is always available (I have no..
	//       .. proof whether or not this is so)
	//       v
	if(FormatMessageA(FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, c, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR)&d, 0, NULL) != 0)
	{
		if(b == NULL)
		{
			*a = strlen(d) + 1;
		}
		else
		{
			strncpy(b, d, *a);
		}
		LocalFree(d);
	}
	else
	{
		if(b == NULL)
		{
			// https://stackoverflow.com/questions/29087129/how-to-calculate-the-length-of-output-that-sprintf-will-generate
			*a = snprintf(NULL, 0, "%u", c) + 1;
		}
		else
		{
			snprintf(b, *a, "%u", c);
		}
	}	
}
#endif

//*****************************************************************************

static int bIsLoaded = 0;

#if defined(_WIN32)
struct win32_t
{
	struct
	{
		HINSTANCE a;
	} hinstance;
};
static struct win32_t win32;
#else //< #elif defined(__linux__)
struct xlib_t
{
	struct
	{
		Display* a;
	} display;
	struct
	{
		int a;
	} screen;
	struct
	{
		Atom a;
	} wmdeletewindow;
	struct
	{
		Atom a;
	} netwmstate;
	struct
	{
		Atom a;
	} netwmstatefullscreen;
};
static struct xlib_t xlib;
#endif

static void(*on_print)(char* a, FILE* b) = NULL;
static void on_printf(FILE* a, char* b, ...)
{
	va_list c;
	va_start(c, b);
	
	int d = vsnprintf(NULL, 0, b, c);
	
	char e[d + 1];
	
	vsprintf(e, b, c);
	
	va_end(c);
	
	on_print(e, a);
}

//*****************************************************************************

void wm_set_on_print(void(*a)(char*, FILE*))
{
	on_print = a;
}
void wm_unset_on_print()
{
	on_print = NULL;
}

#if defined(_WIN32)
enum
{
	ELoadWin32Progress_RegisterClassADefault = 1,
	ELoadWin32Progress_RegisterClassAOpengl = 2
};
#define ELoadWin32Progress_All ELoadWin32Progress_RegisterClassAOpengl

static LRESULT CALLBACK MyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void unload_win32(int progress, struct win32_t* a);
static int load_win32()
{
	struct
	{
		HINSTANCE a;
	} hinstance;

	int progress = 0;
	
	do
	{
		// NOTE: "The operating system uses [hInstance] to identify the..
		//       .. executable (EXE) when it is loaded in memory",
		//       https://learn.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
		//       "If [lpModuleName] is NULL, GetModuleHandle returns a..
		//       .. handle to the file used to create the calling process..
		//       .. (.exe file).",
		//       https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlea
		//       ^
		//       thus both WinMain hInstance is handle to executable and..
		//       .. GetModuleHandleA(NULL) returns handle to executable..?
		// NOTE: https://devblogs.microsoft.com/oldnewthing/20050418-59/?p=35873
		hinstance.a = GetModuleHandleA(NULL);

		WNDCLASSA a;
		a.style = CS_HREDRAW | CS_VREDRAW;
		a.lpfnWndProc = (WNDPROC)&MyWndProc;
		a.cbClsExtra = 0;
		a.cbWndExtra = 0;
		a.hInstance = hinstance.a;
		a.hIcon = NULL;
		a.hCursor = NULL;
		a.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		a.lpszMenuName = NULL;
		a.lpszClassName = "WM_DEFAULT";

		if(RegisterClassA(&a) == 0)
		{
			if(on_print != NULL)
			{
				int c;
				getlasterror_to_string(&c, NULL);
				char d[c];
				getlasterror_to_string(&c, d);

				on_printf(stderr, "error: %s in %s\n", d, __FUNCTION__);
			}
			
			break;
		}
		
		progress = ELoadWin32Progress_RegisterClassADefault;
		
		// https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
		
		WNDCLASS b;
		b.style = CS_OWNDC;
		b.lpfnWndProc = &MyWndProc;
		b.cbClsExtra = 0;
		b.cbWndExtra = 0;
		b.hInstance = win32.hinstance.a;
		b.hIcon = NULL;
		b.hCursor = NULL;
		b.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
		b.lpszMenuName = NULL;
		b.lpszClassName = "WM_OPENGL";
		
		if(RegisterClassA(&b) == 0)
		{
			if(on_print != NULL)
			{
				int c;
				getlasterror_to_string(&c, NULL);
				char d[c];
				getlasterror_to_string(&c, d);
				
				on_printf(stderr, "error: %s in %s\n", d, __FUNCTION__);
			}
			
			break;
		}
		
		progress = ELoadWin32Progress_RegisterClassAOpengl;
	} while(0);
	if(progress != ELoadWin32Progress_All)
	{
		struct win32_t a;
		a.hinstance.a = hinstance.a;
		unload_win32(progress, &a);
		
		return 0;
	}
	
	win32.hinstance.a = hinstance.a;
	
	return 1;
}
static void unload_win32(int progress, struct win32_t* a)
{
	if(progress >= ELoadWin32Progress_RegisterClassAOpengl)
	{
		if(UnregisterClassA("WM_OPENGL", a->hinstance.a) == 0)
		{
			if(on_print != NULL)
			{
				int b;
				getlasterror_to_string(&b, NULL);
				char c[b];
				getlasterror_to_string(&b, c);
				
				on_printf(stdout, "warning: %s in %s\n", c, __FUNCTION__);
			}
		}
	}
	if(progress >= ELoadWin32Progress_RegisterClassADefault)
	{
		if(UnregisterClassA("WM_DEFAULT", a->hinstance.a) == 0)
		{
			if(on_print != NULL)
			{
				int b;
				getlasterror_to_string(&b, NULL);
				char c[b];
				getlasterror_to_string(&b, c);
				
				on_printf(stdout, "warning: %s in %s\n", c, __FUNCTION__);
			}
		}
	}
}
#else //< #elif defined(__linux__)
enum
{
	ELoadXlibProgress_XInitThreads = 1,
	ELoadXlibProgress_XOpenDisplay = 2,
	ELoadXlibProgress_XInternAtomWmdeletewindow = 3,
	ELoadXlibProgress_XInternAtomNetwmstate = 4,
	ELoadXlibProgress_XInternAtomNetwmstatefullscreen = 5
};
#define ELoadXlibProgress_All ELoadXlibProgress_XInternAtomNetwmstatefullscreen

static void unload_xlib(int progress, struct xlib_t* a);
static int load_xlib()
{
	struct
	{
		Display* a;
	} display;
	
	struct
	{
		Atom a;
	} wmdeletewindow;
	struct
	{
		Atom a;
	} netwmstate;
	struct
	{
		Atom a;
	} netwmstatefullscreen;

	int progress = 0;

	do
	{
		if(XInitThreads() == 0)
		{
			if(on_print != NULL)
			{
				on_printf(stderr, "error: XInitThreads() == 0 in %s\n", __FUNCTION__);
			}
			break;
		}
		
		progress = ELoadXlibProgress_XInitThreads;
		
		display.a = XOpenDisplay(NULL);
		if(display.a == NULL)
		{
			if(on_print != NULL)
			{
				on_printf(stderr, "error: XOpenDisplay(NULL) == NULL in %s\n", __FUNCTION__);
			}
			break;
		}
		
		progress = ELoadXlibProgress_XOpenDisplay;
		
		// https://stackoverflow.com/questions/1157364/intercept-wm-delete-window-on-x11
		wmdeletewindow.a = XInternAtom(display.a, "WM_DELETE_WINDOW", False);
		// NOTE: "If only_if_exists is False, the atom is created if it does..
		//       .. not exist. Therefore, XInternAtom can return None.",..
		//       .. https://www.x.org/releases/X11R7.5/doc/man/man3/XInternAtom.3.html
		//       ^
		//       not sure if any other cause than memory allocation failure..
		//       .. can cause XInternAtom to return None here
		if(wmdeletewindow.a = None)
		{
			break;
		}
		
		progress = ELoadXlibProgress_XInternAtomWmdeletewindow;
		
		netwmstate.a = XInternAtom(display.a, "_NET_WM_STATE", False);
		if(netwmstate.a = None)
		{
			break;
		}
		
		progress = ELoadXlibProgress_XInternAtomNetwmstate;
		
		netwmstatefullscreen.a = XInternAtom(display.a, "_NET_WM_STATE_FULLSCREEN", False);
		if(netwmstatefullscreen.a = None)
		{
			break;
		}
		
		progress = ELoadXlibProgress_XInternAtomNetwmstatefullscreen;
	} while(0);
	if(progress != ELoadXlibProgress_All)
	{
		struct xlib_t a;
		a.display = display.a;
		unload_xlib(progress, &a);
		
		return 0;
	}
	
	xlib.display.a = display.a;
	xlib.screen.a = DefaultScreen(xlib.display.a);
	// NOTE: ^
	//       no mention of possibility that DefaultScreen fails thus assumed..
	//       .. here that it wont,..
	//       .. https://www.x.org/releases/X11R7.5/doc/libX11/libX11.html
	// NOTE: "Client applications can display overlapping and nested windows..
	//       .. on one or more screens",..
	//       .. https://www.x.org/releases/X11R7.5/doc/libX11/libX11.html
	//       ^
	//       thus not sure if more than one screen from a single display is..
	//       .. possible, but guaranteed that it isn't using DefaultScreen..
	//       .. as "[DefaultScreen] return[s] the default screen number..
	//       .. referenced by the XOpenDisplay function. This macro [...]..
	//       .. should be used to retrieve the screen number in applications..
	//       .. that will use only a single screen.", https://www.x.org/releases/X11R7.5/doc/libX11/libX11.html
	// TODO: ^
	//       test whether it is possible to move between screens using..
	//       .. XScreenCount
	//       ^
	//       OR not sure if OS only uses a single screen across all monitors?
	
	xlib.wmdeletewindow.a = wmdeletewindow.a;
	xlib.netwmstate.a = netwmstate.a;
	xlib.netwmstatefullscreen.a = netwmstatefullscreen.a;

	return 1;
}
static void unload_xlib(int progress, struct xlib_t* a)
{
	if(progress >= ELoadXlibProgress_XOpenDisplay)
	{
		XCloseDisplay(a->display.a);
		// ^
		// "XCloseDisplay can generate a BadGC error.",..
		// .. https://www.x.org/releases/X11R7.5/doc/libX11/libX11.html
		// ^
		// not sure if only if display.a is invalid
		// any way currently no use of xlib error handler meaning "When Xlib..
		// .. detects an error, it calls an error handler, which your..
		// .. program can provide. If you do not provide an error handler,..
		// .. the error is printed, and your program terminates.",..
		// .. https://www.x.org/releases/X11R7.5/doc/libX11/libX11.html
	}
}
#endif

int wm_load()
{
	if(bIsLoaded == 1)
	{
		return -1;
	}

#if defined(_WIN32)
	if(load_win32() != 1)
	{
		return 0;
	}
#else //< #elif defined(__linux__)
	if(load_xlib() != 1)
	{
		return 0;
	}
#endif

	bIsLoaded = 1;

	return 1;
}

static int numWindows;
// ^
// is valid?
// ^
// https://stackoverflow.com/a/24783091
int wm_unload()
{
	if(bIsLoaded == 0)
	{
		return -1;
	}
	
	if(numWindows > 0)
	{
		if(on_print != NULL)
		{
			on_printf(stdout, "warning: numWindows > 0 in %s\n", __FUNCTION__);
		}
		return -1;
	}

#if defined(_WIN32)
	unload_win32(ELoadWin32Progress_All, &win32);
#else //< #elif defined(__linux__)
	unload_xlib(ELoadXlibProgress_All, &xlib);
#endif

	bIsLoaded = 0;

	return 1;
}

int wm_get_info(struct wm_info_t* info)
{
	if(bIsLoaded == 0)
	{
		return -1;
	}
	
#if defined(_WIN32)
	info->win32.hinstance.a = win32.hinstance.a;
#else //< #elif defined(__linux__)
	info->xlib.display.a = win32.xlib.display.a;
#endif
	
	return 1;
}

//*****************************************************************************

struct info_about_window_t
{
	struct wm_window_source_t* source;
	int minWidthInPixels;
	int minHeightInPixels;
	int maxWidthInPixels;
	int maxHeightInPixels;
	int widthInPixels;
	int heightInPixels;
	char* title;
	int bFullscreen;
	int bIsResizing;
	struct
	{
		int newWidthInPixels;
		int newHeightInPixels;
	} ifResizing;
	// as there are multiple places where resizing can be triggered use..
	// .. variable to only write resizing callback call + <width and height>..
	// .. updating once
	// v
	int bResize;
	//int addWindowParametersFlags;
	//int bIsPaused;
	//int bResize;
#if defined(_WIN32)
	int bIsMinimized;
	int bIsMaximized;
	struct
	{
		struct
		{
			HWND a;
		} hwnd;
		struct
		{
			WINDOWPLACEMENT a;
		} windowplacement;
	} win32;
#else //< #elif defined(__linux__)
	int bIsFocused;
	int bClose;
	// ^
	// bClose here because processing close in message loop allows for..
	// .. continue
	struct
	{
		struct
		{
			Window a;
		} window;
	} xlib;
#endif
};
static struct info_about_window_t info_about_window_default = { .bIsResizing = 0, .bResize = 0
#if defined(_WIN32)
		,
		.bIsMinimized = 0,
		.bIsMaximized = 0
#else //< #elif defined(__linux__)
		,
		.bIsFocused = 0,
		.bClose = 0
#endif
	};

static int numWindowsTheresRoomFor = 0;
static int numWindows = 0;
static struct info_about_window_t* infoPerWindow;

static int(*on_window_closed)(int window);
static void(*on_window_resized)(int window, int widthInPixels, int heightInPixels);
static void(*on_window_focused)(int window);
static void(*on_window_unfocused)(int window);

//*****************************************************************************

void wm_set_on_window_closed(int(*a)(int))
{
	on_window_closed = a;
}
void wm_unset_on_window_closed()
{
	on_window_closed = NULL;
}
void wm_set_on_window_resized(void(*a)(int, int, int))
{
	on_window_resized = a;
}
void wm_unset_on_window_resized()
{
	on_window_resized = NULL;
}
void wm_set_on_window_focused(void(*a)(int))
{
	on_window_focused = a;
}
void wm_unset_on_window_focused()
{
	on_window_focused = NULL;
}
void wm_set_on_window_unfocused(void(*a)(int))
{
	on_window_unfocused = a;
}
void wm_unset_on_window_unfocused()
{
	on_window_unfocused = NULL;
}

#ifdef __linux__
int toggle_fullscreen(struct info_about_window_t* infoAboutWindow);
#endif
// NOTE: assumes..
//       .. infoAboutWindow->bFullscreen == 0
static int start_fullscreen(struct info_about_window_t* infoAboutWindow)
{
#if defined(_WIN32)
	// NOTE: information about fullscreen..
	//       .. https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
	
	if (GetWindowPlacement(infoAboutWindow->win32.hwnd.a, &infoAboutWindow->win32.windowplacement.a) == 0)
	{
		int c;
		getlasterror_to_string(&c, NULL);
		char d[c];
		getlasterror_to_string(&c, d);
		
		on_printf(stdout, "warning: %s in %s\n", d, __FUNCTION__);
		
		return 0;
	}
	
	MONITORINFO a;
	a.cbSize = sizeof(MONITORINFO);
	if(GetMonitorInfo(MonitorFromWindow(infoAboutWindow->win32.hwnd.a, MONITOR_DEFAULTTOPRIMARY), &a) == 0)
	{
		// NOTE: not sure if GetLastError is used by GetMonitorInfoA..
		//       .. (documentation doesn't mention)
		//       ^
		//       https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmonitorinfoa
		
		int c;
		getlasterror_to_string(&c, NULL);
		char d[c];
		getlasterror_to_string(&c, d);
		
		on_printf(stdout, "warning: %s in %s\n", d, __FUNCTION__);
		
		return 0;
	}

	DWORD b = GetWindowLong(infoAboutWindow->win32.hwnd.a, GWL_STYLE);
	SetWindowLong(infoAboutWindow->win32.hwnd.a, GWL_STYLE, b & ~WS_OVERLAPPEDWINDOW);
	SetWindowPos(infoAboutWindow->win32.hwnd.a, HWND_TOP, a.rcMonitor.left, a.rcMonitor.top, a.rcMonitor.right - a.rcMonitor.left, a.rcMonitor.bottom - a.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	
	infoAboutWindow->bFullscreen = 1;
	
	return 1;
#else //< #elif defined(__linux__)
	return toggle_fullscreen(infoAboutWindow);
#endif
}
// NOTE: assumes..
//       .. infoAboutWindow->bFullscreen == 1
static int stop_fullscreen(struct info_about_window_t* infoAboutWindow)
{
#if defined(_WIN32)
	// NOTE: information about fullscreen..
	//       .. https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
	
	DWORD a = GetWindowLong(infoAboutWindow->win32.hwnd.a, GWL_STYLE);
	
	SetWindowLong(infoAboutWindow->win32.hwnd.a, GWL_STYLE, a | WS_OVERLAPPEDWINDOW);
	SetWindowPlacement(infoAboutWindow->win32.hwnd.a, &infoAboutWindow->win32.windowplacement.a);
	SetWindowPos(infoAboutWindow->win32.hwnd.a, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	
	infoAboutWindow->bFullscreen = 0;
	
	return 1;
#else //< #elif defined(__linux__)
	return toggle_fullscreen(infoAboutWindow);
#endif	
}
static int toggle_fullscreen(struct info_about_window_t* infoAboutWindow)
{
#if defined(_WIN32)
	if(infoAboutWindow->bFullscreen == 0)
	{
		return start_fullscreen(infoAboutWindow);
	}
	else
	{
		return stop_fullscreen(infoAboutWindow);
	}
#else //< #elif defined(__linux__)
	XEvent a;
	a.type = ClientMessage;
	a.xclient.serial = 0;
	a.xclient.send_event = True;
	a.xclient.display = xlib.display.a;
	a.xclient.window = infoAboutWindow->xlib.window.a;
	a.xclient.message_type = xlib.netwmstate.a;
	a.xclient.format = 32;
	a.xclient.data.l[0] = 2;
	a.xclient.data.l[1] = xlib.netwmstatefullscreen.a;
	a.xclient.data.l[2] = 0;
	a.xclient.data.l[3] = 0;
	a.xclient.data.l[4] = 0;
	
	XSendEvent(xlib.display.a, RootWindow(xlib.display.a, xlib.screen.a), False, StructureNotifyMask | ResizeRedirectMask, &a);
	
	// NOTE: information about _NET_WM_STATE_FULLSCREEN..
	//       https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html#idm46435610051248
	// NOTE: example of using _NET_WM_STATE_FULLSCREEN..
	//       .. https://mail.gnome.org/archives/metacity-devel-list/2010-February/msg00000.html
	
	infoAboutWindow->bFullscreen = infoAboutWindow->bFullscreen == 0 ? 1 : 0;
	
	return 1;
#endif
}

#if defined(_WIN32)
enum
{
	EAddInfoAboutWindowWin32Progress_CreateWindowA = 1
};
#define EAddInfoAboutWindowWin32Progress_All EAddInfoAboutWindowWin32Progress_CreateWindowA

static void remove_info_about_window_win32(int progress, struct info_about_window_t* a);
static int add_info_about_window_win32(struct wm_add_window_parameters_t* parameters, struct wm_window_source_t* source, struct info_about_window_t* infoAboutWindow)
{
	struct
	{
		HWND a;
	} hwnd;

	int progress = 0;
	
	do
	{
		// NOTE: "The window is an overlapped window. Same as the WS_TILEDWINDOW..
		//       .. style."
		//       ^
		//       https://learn.microsoft.com/en-us/windows/win32/winmsg/window-styles
		// NOTE: "If the y parameter is CW_USEDEFAULT, then the window..
		//       .. manager calls ShowWindow with the SW_SHOW flag after the..
		//       .. window has been created.",
		//       https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa
		//       "[SW_SHOW] activates the window and displays it in its current size..
		//       .. and position.",
		//       https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
		//       ^
		//       CW_USEDEFAULT would ignore nCmdShow	// NOTE: "If an overlapped window is created with the WS_VISIBLE style..
		//       .. bit set and the x parameter is set to CW_USEDEFAULT, then..
		//       .. the y parameter determines how the window is shown. [...] If..
		//       .. the y parameter is [not CW_USEDEFAULT], then the window..
		//       .. manager calls ShowWindow with that value as the nCmdShow..
		//       .. parameter.",
		//       https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowa
		//       "[SW_SHOWDEFAULT s]ets the show state based on the SW_..
		//       .. value specified in the STARTUPINFO structure passed to..
		//       .. the CreateProcess function by the program that started..
		//       .. the application.",
		//       https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
		//       ^
		//       not sure if correct as not sure if every window should use..
		//       .. <STARTUPINFOA>.wShowWindow
		// NOTE: "[nCmdShow] is ignored the first time an application calls..
		//       .. ShowWindow, if the program that launched the application..
		//       .. provides a STARTUPINFO structure.",
		//       https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow
		//       STARTUPINFO is not optional in CreateProcessA
		//       ^
		//       "[in] lpStartupInfo",
		//       https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa
		//       ^
		//       thus if ShowWindowA is not called before CreateWindowA the y..
		//       .. parameter to CreateWindowA will be ignored..?
		//       ^
		//       not sure if mistake in documentation
		//       ^
		//       https://github.com/MicrosoftDocs/feedback/issues/3890
		
		DWORD a = WS_TILEDWINDOW | WS_VISIBLE; //< CreateWindowA dwStyle
		int b; //< CreateWindowA nWidth
		int c; //< CreateWindowA nHeight
		if((source->widthInPixels != -1) | (source->heightInPixels != -1))
		{
			RECT e;
			e.left = 0;
			e.top = 0;
			e.right = source->widthInPixels;
			e.bottom = source->heightInPixels;
			if(AdjustWindowRect(&e, a, FALSE) == 0)
			{
				if(on_print != NULL)
				{
					int f;
					getlasterror_to_string(&f, NULL);
					char g[f];
					getlasterror_to_string(&f, g);
					
					on_printf(stdout, "warning: %s in %s\n", g, __FUNCTION__);
				}
			}
			b = source->widthInPixels == -1 ? CW_USEDEFAULT : e.right - e.left;
			c = source->heightInPixels == -1 ? CW_USEDEFAULT : e.bottom - e.top;
		}
		else
		{
			b = CW_USEDEFAULT;
			c = CW_USEDEFAULT;
		}

		// NOTE: "client coordinates are relative to the upper-left corner..
		//       .. of a window's client area",..
		//       .. https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect
		// NOTE: documentation is vague about whether b.top and b.left are..
		//       .. not 0 or not here.. "the structure contains the..
		//       .. coordinates of the top-left and bottom-right corners of..
		//       .. the window to accommodate the desired client area",..
		//       .. https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-adjustwindowrect
		//       ^
		//       probably if b.top and b.left were both 0 before..
		//       .. AdjustWindowRect will now be both negative?
		char* d = (parameters->flags & EWMAddWindowParametersFlag_Opengl) != 0 ? "WM_OPENGL" : "WM_DEFAULT";
		hwnd.a = CreateWindowA(d, source->title, a, CW_USEDEFAULT, SW_SHOWDEFAULT, b, c, NULL, NULL, win32.hinstance.a, NULL);
		if(hwnd.a == NULL)
		{
			if(on_print != NULL)
			{
				int e;
				getlasterror_to_string(&e, NULL);
				char f[e];
				getlasterror_to_string(&e, f);
				
				on_printf(stderr, "error: %s in %s\n", f, __FUNCTION__);
			}
			
			break;
		}
		
		progress = EAddInfoAboutWindowWin32Progress_CreateWindowA;
	} while(0);
	if(progress != EAddInfoAboutWindowWin32Progress_All)
	{
		struct info_about_window_t a;
		a.win32.hwnd.a = hwnd.a;
		remove_info_about_window_win32(progress, &a);
		
		return 0;
	}
	
	infoAboutWindow->win32.hwnd.a = hwnd.a;
	
	return 1;
}
#else //< #elif defined(__linux__)
enum
{
	EAddInfoAboutWindowXlibProgress_XCreateWindow = 1,
	EAddInfoAboutWindowXlibProgress_XSetWMProtocols = 2
};
#define EAddInfoAboutWindowXlibProgress_All EAddInfoAboutWindowXlibProgress_XSetWMProtocols

static int add_info_about_window_xlib(struct wm_add_window_parameters_t* parameters, struct wm_window_source_t* source, struct info_about_window_t* a)
{
	struct
	{
		Window a;
	} window;
	
	int progress = 0;
	
	do
	{
		// TODO: read xInPixels and yInPixels from file (i.e. restore previous)..
		//       .. as this appears to be the default on linux..?
		int xInPixels = (XDisplayWidth(xlib.display.a, xlib.screen.a) / 2.0f) - (source->widthInPixels / 2.0f);
		int yInPixels = (XDisplayHeight(xlib.display.a, xlib.screen.a) / 2.0f) - (source->heightInPixels / 2.0f);
		// ^
		// "XDisplayWidth, XDisplayHeight [...] really should be named..
		// .. Screenwhatever and XScreenwhatever",..
		// .. https://x.org/releases/current/doc/libX11/libX11/libX11.html

		XSetWindowAttributes a;
		a.background_pixel = BlackPixel(xlib.display.a, xlib.screen.a);
		a.border_pixel = WhitePixel(xlib.display.a, xlib.screen.a);
		a.event_mask = FocusChangeMask | StructureNotifyMask;

		// TODO: XSynchronize + XSetAfterFunction + XSetErrorHandler +..
		//       .. X_CreateWindow (from X11/Xproto.h) <- turn on..
		//       .. synchronization before adding and turn off after added..?
		//       ^
		//       https://cgit.freedesktop.org/xorg/proto/xproto/tree/Xproto.h
		window.a = XCreateWindow(xlib.display.a, RootWindow(xlib.display.a, xlib.screen.a), xInPixels, yInPixels, source->widthInPixels, source->heightInPixels, 1, CopyFromParent, CopyFromParent, CopyFromParent, CWBackPixel | CWBorderPixel | CWEventMask, &a);
		// NOTE: ^
		//       seems to completely ignore x and y
		
		progress = EAddInfoAboutWindowXlibProgress_XCreateWindow;
		
		if(source->title != NULL)
		{
			XStoreName(xlib.display.a, window.a, source->title);
		}
		
		// NOTE: "If it cannot intern the WM_PROTOCOLS atom, XSetWMProtocols..
		//       .. returns a zero status. Otherwise, it returns a nonzero..
		//       .. status.",..
		//       .. https://x.org/releases/current/doc/libX11/libX11/libX11.html#XCreateWindow
		// NOTE: https://stackoverflow.com/questions/1157364/intercept-wm-delete-window-on-x11
		if(XSetWMProtocols(xlib.display.a, window.a, &xlib.wmdeletewindow.a, 1) == 0)
		{
			if(on_print != NULL)
			{
				on_printf("error: XSetWMProtocols == 0 in %s\n", __FUNCTION__);
			}
			break;
		}
		
		progress = EAddInfoAboutWindowXlibProgress_XSetWMProtocols;
		
		XMapWindow(xlib.display.a, window.a);
	} while(0);
	if(progress != EAddInfoAboutWindowXlibProgress_All)
	{
		struct info_about_window_t a;
		a.xlib.window.a = window.a;
		remove_info_about_window_xlib(progress, &a);
		
		return 0;
	}
	
	infoAboutWindow->xlib.window.a = window.a;
	
	return 1;
}
#endif
static int add_info_about_window(struct wm_add_window_parameters_t* parameters, struct wm_window_source_t* source, struct info_about_window_t* infoAboutWindow)
{
#if defined(_WIN32)
	if(add_info_about_window_win32(parameters, source, infoAboutWindow) != 1)
	{
		return 0;
	}
#else //< #elif defined(__linux__)
	if(add_info_about_window_xlib(parameters, source, infoAboutWindow) != 1)
	{
		return 0;
	}
#endif

	infoAboutWindow->source = source;
	infoAboutWindow->minWidthInPixels = source->minWidthInPixels;
	infoAboutWindow->minHeightInPixels = source->minHeightInPixels;
	infoAboutWindow->maxWidthInPixels = source->maxWidthInPixels;
	infoAboutWindow->maxHeightInPixels = source->maxHeightInPixels;
	infoAboutWindow->widthInPixels = source->widthInPixels;
	infoAboutWindow->heightInPixels = source->heightInPixels;
	if(source->title != NULL)
	{
		int titleLength = strlen(source->title);
		//infoAboutWindow->title = new char[titleLength + 1];
		infoAboutWindow->title = malloc(titleLength + 1);
		memcpy(infoAboutWindow->title, source->title, titleLength + 1);
	}
	else
	{
		infoAboutWindow->title = NULL;
	}
	infoAboutWindow->bFullscreen = 0;
	if(source->bFullscreen == 1)
	{
		start_fullscreen(infoAboutWindow);
	}
	infoAboutWindow->ifResizing.newWidthInPixels = infoAboutWindow->widthInPixels;
	infoAboutWindow->ifResizing.newHeightInPixels = infoAboutWindow->heightInPixels;
	
	return 1;
}
int wm_add_window(struct wm_add_window_parameters_t* parameters, struct wm_window_source_t* source, int* window)
{
	if(bIsLoaded == 0)
	{
		return -1;
	}
	
	int indexToWindow;
	if(numWindowsTheresRoomFor > numWindows)
	{
		for(int i = 0; i < numWindowsTheresRoomFor; ++i)
		{
			if(infoPerWindow[i].source == NULL)
			{
				indexToWindow = i;
				break;
			}
		}
	}
	else
	{
		add_or_append_one_element2(&numWindowsTheresRoomFor, &infoPerWindow);
		indexToWindow = numWindowsTheresRoomFor - 1;
	}
	
	struct info_about_window_t a = info_about_window_default;
	
	if(add_info_about_window(parameters, source, &a) != 1)
	{
		return 0;
	}
	
	struct info_about_window_t* infoAboutWindow = &infoPerWindow[indexToWindow];
	*infoAboutWindow = a;
	
	++numWindows;
	
	*window = indexToWindow;
	
	return 1;
}

static int is_window(int window)
{
	if((window < 0) | (window >= numWindowsTheresRoomFor))
	{
		return 0;
	}
	return infoPerWindow[window].source == NULL ? 0 : 1;
}

int wm_edit_window(int window, int conditions)
{
	//...

	return 1;
}

#if defined(_WIN32)
static void remove_info_about_window_win32(int progress, struct info_about_window_t* a)
{
	if(progress >= EAddInfoAboutWindowWin32Progress_CreateWindowA)
	{
		// NOTE: tested and WM_ACTIVATE was sent in DestroyWindow which..
		//       .. caused on_focused callback to be called, to prevent this..
		//       .. set a->win32.hwnd.a to NULL to cause window == -1 in..
		//       .. MyWndProc and thus uMsg to be ignored
		//       v
		struct
		{
			HWND a;
		} hwnd = { .a = a->win32.hwnd.a };
		a->win32.hwnd.a = NULL;
		
		// NOTE: see image at..
		//       https://learn.microsoft.com/en-us/windows/win32/learnwin32/closing-the-window
		//       v
		//       causes WM_DESTROY
		//       v
		if(DestroyWindow(hwnd.a) == 0)
		{
			if(on_print != NULL)
			{
				int b;
				getlasterror_to_string(&b, NULL);
				char c[b];
				getlasterror_to_string(&b, c);
				
				on_printf(stdout, "warning: %s in %s\n", c, __FUNCTION__);
			}
		}
		
		// tested and DestroyWindow called MyWndProc with WM_DESTROY thus..
		// .. below should not be required
		// v
		/*
		MSG b;
		//BOOL c = GetMessage(&b, hwnd.a, WM_DESTROY, WM_DESTROY);
		// NOTE: ^
		//       "During the processing of [WM_DESTROY], it can be assumed..
		//       .. that all child windows still exist.",..
		//       .. https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-destroy
		//       ^
		//       tested above, but causes error "Invalid window handle."
		// NOTE: not sure if loop is required here, but to be safe in case..
		//       .. it is possible for a rogue WM_DESTROY (or multiple) to..
		//       .. be lingering here
		//       v
		while(PeekMessage(&b, NULL, WM_DESTROY, WM_DESTROY, PM_NOREMOVE) != 0)
		{
			BOOL c = GetMessage(&b, NULL, WM_DESTROY, WM_DESTROY);
			if(c <= 0)
			{
				if(c == -1)
				{
					if(on_print != NULL)
					{
						int d;
						getlasterror_to_string(&d, NULL);
						char e[d];
						getlasterror_to_string(&d, e);
						
						on_printf(stdout, "warning: %s in %s\n", e, __FUNCTION__);
					}
				}
				else
				{
					// WM_QUIT
					
					if(on_print != NULL)
					{
						on_printf(stdout, "warning: WM_QUIT received which window-mini does not support in %s\n", __FUNCTION__);
					}
				}
			}
		}
		*/
	}
}
#else //< #elif defined(__linux__)
static void remove_info_about_window_xlib(int progress, struct info_about_window_t* a)
{
	if(progress >= EAddInfoAboutWindowXlibProgress_XCreateWindow)
	{
		XDestroyWindow(xlib.display.a, a->xlib.window.a);		
	}
}
#endif
void remove_info_about_window(struct info_about_window_t* a)
{
#if defined(_WIN32)
	remove_info_about_window_win32(EAddInfoAboutWindowWin32Progress_All, a);
#else //< #elif defined(__linux__)
	remove_info_about_window_xlib(EAddInfoAboutWindowXlibProgress_All, a);
#endif
	
	if(a->title != NULL)
	{
		//delete a->title;
		free(a->title);
	}
}
int wm_remove_window(int window)
{
	if(is_window(window) == 0)
	{
		return -1;
	}
	
	--numWindows;

	struct info_about_window_t* a = &infoPerWindow[window];

	remove_info_about_window(a);
	
	if(window == numWindowsTheresRoomFor - 1)
	{
		int indexToPreviousWindow = -1;
		for(int i = window - 1; i >= 0; --i)
		{
			if(infoPerWindow[i].source != NULL)
			{
				indexToPreviousWindow = i;
				break;
			}
		}
		
		int lastNumElementsToRemove = (numWindowsTheresRoomFor - 1) - indexToPreviousWindow;
		remove_last_num_elements2(&numWindowsTheresRoomFor, &infoPerWindow, lastNumElementsToRemove);
	}
	else
	{
		a->source = NULL;
	}

	return 1;
}

int wm_get_info_about_window(int window, struct wm_info_about_window_t* infoAboutWindow)
{
	if(is_window(window) == 0)
	{
		return -1;
	}

	struct info_about_window_t* a = &infoPerWindow[window];

#if defined(_WIN32)
	infoAboutWindow->win32.hwnd.a = a->win32.hwnd.a;
#else //< #elif defined(__linux__)
	infoAboutWindow->xlib.window.a = a->xlib.window.a;
#endif
	
	return 1;
}

//*****************************************************************************

static int should_window_be_resized(struct info_about_window_t* infoAboutWindow)
{
	int bWidthIsDifferent = infoAboutWindow->widthInPixels != infoAboutWindow->ifResizing.newWidthInPixels ? 1 : 0;
	int bHeightIsDifferent = infoAboutWindow->heightInPixels != infoAboutWindow->ifResizing.newHeightInPixels ? 1 : 0;
	return bWidthIsDifferent | bHeightIsDifferent;
}

static struct
{
	int bWasAnyWindowClosed; //< at the end of wm_poll if bWasAnyWindowClosed == 1.. will update numWindowsTheresRoomFor, numWindows and infoPerWindow
} infoAboutPoll;
#if defined(_WIN32)
static LRESULT CALLBACK MyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//printf("MyWndProc %i, %u, %i, %i\n", hwnd, uMsg, wParam, lParam);

	int window = -1;
	for(int i = 0; i < numWindowsTheresRoomFor; ++i)
	{
		if(infoPerWindow[i].source == NULL)
		{
			continue;
		}
		
		if(infoPerWindow[i].win32.hwnd.a == hwnd)
		{
			window = i;
			break;
		}
	}
	//printf("window == %i\n", window);
	if(window == -1)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	
	struct info_about_window_t* infoAboutWindow = &infoPerWindow[window];
	
	switch(uMsg)
	{
	case WM_CLOSE:
		// must be done here as after GetMessage a.hwnd will == NULL thus..
		// .. cannot determine which window was closed there
		// v
		if(on_window_closed != NULL)
		{
			if(on_window_closed(window) == 0)
			{
				break;
			}
		}
		
		infoAboutWindow->source = NULL;
		
		infoAboutPoll.bWasAnyWindowClosed = 1;
		
		//remove_info_about_window(infoAboutWindow);
		// ^
		// would work, but still not sure if DestroyWindow call..
		// .. functionality changes if called from MyWndProc or not
		// code here is based on this reference..
		// .. https://learn.microsoft.com/en-us/windows/win32/learnwin32/closing-the-window
		
		// NOTE: see image at..
		//       https://learn.microsoft.com/en-us/windows/win32/learnwin32/closing-the-window
		//       v
		//       causes WM_DESTROY
		//       v
		if(DestroyWindow(hwnd) == 0)
		{
			if(on_print != NULL)
			{
				int a;
				getlasterror_to_string(&a, NULL);
				char b[a];
				getlasterror_to_string(&a, b);
				
				on_printf(stderr, "error: %s in %s\n", b, __FUNCTION__);
			}
		}
		
		if(infoAboutWindow->title != NULL)
		{
			//delete infoAboutWindow->title;
			free(infoAboutWindow->title);
		}
		break;
	/*
	case WM_DESTROY:
		//PostQuitMessage(0);
		// NOTE: ^
		//       don't call PostQuitMessage as "PostQuitMessage puts a..
		//       .. WM_QUIT message on the message queue, causing the..
		//       .. message loop to end",..
		//       .. https://learn.microsoft.com/en-us/windows/win32/learnwin32/closing-the-window
		break;
		// NOTE: ^
		//       processing WM_DESTROY is not required as close from GUI is..
		//       .. handled using WM_CLOSE and doesn't require processing..
		//       .. WM_DESTROY and close from code doesn't require..
		//       .. processing WM_CLOSE nor WM_DESTROY
	*/
	case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE)
		{
			if(on_window_unfocused != NULL)
			{
				on_window_unfocused(window);
			}
		}
		else
		{
			if(on_window_focused != NULL)
			{
				on_window_focused(window);
			}
		}
		break;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE); //< tested.. if was in fullscreen then exit fullscreen w. alt+enter.. without WM_MENUCHAR sound on exit, with WM_MENUCHAR no sound on exit
	case WM_SIZE:
		infoAboutWindow->ifResizing.newWidthInPixels = LOWORD(lParam);
		infoAboutWindow->ifResizing.newHeightInPixels = HIWORD(lParam);
		
		// NOTE: assuming that WM_SIZE only gets sent in DispatchMessage
		//       ^
		//       not sure what this note is about anymore
		switch(wParam)
		{
		case SIZE_MINIMIZED:
			infoAboutWindow->bIsMinimized = 1;
			break;
		case SIZE_MAXIMIZED:
			if(should_window_be_resized(infoAboutWindow) == 1)
			{
				infoAboutWindow->bResize = 1;
			}
			infoAboutWindow->bIsMaximized = 1;
			break;
		case SIZE_RESTORED:
			if(infoAboutWindow->bIsMinimized == 1)
			{
				infoAboutWindow->bIsMinimized = 0;
			}
			else if(infoAboutWindow->bIsMaximized == 1)
			{
				if(should_window_be_resized(infoAboutWindow) == 1)
				{
					infoAboutWindow->bResize = 1;
				}
				infoAboutWindow->bIsMaximized = 0;
			}
			else if(infoAboutWindow->bIsResizing == 1)
			{
				//... //< tested.. got triggered repeatedly if resizing
			}
			else
			{
				if(should_window_be_resized(infoAboutWindow) == 1)
				{
					infoAboutWindow->bResize = 1;
				}
			}
			break;
		};
		break;
	case WM_ENTERSIZEMOVE:
		infoAboutWindow->bIsResizing = 1;
		break;
	case WM_EXITSIZEMOVE:
		// NOTE: not sure whether this makes sense here as only WM_SIZE can..
		//       .. actually update infoAboutWindow->ifResizing.new*InPixels?
		//       v
		if(should_window_be_resized(infoAboutWindow) == 1)
		{
			infoAboutWindow->bResize = 1;
		}
		infoAboutWindow->bIsResizing = 0;
		break;
	case WM_GETMINMAXINFO:
		// https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-minmaxinfo
		// v
		// not sure if not setting pt*TrackSize.* would result in the same..
		// .. as calling GetSystemMetrics manually here, documentation..
		// .. doesn't mention what the default value of each pt*TrackSize is
		// v
		int minWidthInPixels = infoAboutWindow->minWidthInPixels == -1 ? GetSystemMetrics(SM_CXMINTRACK) : infoAboutWindow->minWidthInPixels;
		int minHeightInPixels = infoAboutWindow->minHeightInPixels == -1 ? GetSystemMetrics(SM_CYMINTRACK) : infoAboutWindow->minHeightInPixels;
		int maxWidthInPixels = infoAboutWindow->maxWidthInPixels == -1 ? GetSystemMetrics(SM_CXMAXTRACK) : infoAboutWindow->maxWidthInPixels;
		int maxHeightInPixels = infoAboutWindow->maxHeightInPixels == -1 ? GetSystemMetrics(SM_CYMAXTRACK) : infoAboutWindow->maxHeightInPixels;
		// ^
		// not sure if the value returned by GetSystemMetrics(SM_C*TRACK)..
		// .. will be changed after WM_GETMINMAXINFO returns as "A window..
		// .. can override this value by processing the WM_GETMINMAXINFO..
		// .. message.",..
		// .. https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmetrics
		
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = minWidthInPixels;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = minHeightInPixels;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = maxWidthInPixels;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = maxHeightInPixels;
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}
#else //< #elif defined(__linux__)
static void my_on_xevent(int window, XEvent a)
{
	struct info_about_window_t* infoAboutWindow = &infoPerWindow[window];

	// NOTE: grabbing border for resizing window causes FocusOut on mouse..
	//       .. button down + FocusIn on mouse button up
	if((XGetEventData(xlib.display.a, &a.xcookie) != False) & (a.xcookie.type == GenericEvent))
	{
		struct
		{
			XIEvent* a;
		} xievent;
		xievent.a = (XIEvent*)a.xcookie.data;
		my_on_xievent(xievent.a);
	}
	XFreeEventData(xlib.display.a, &a.xcookie);

	switch(a.type)
	{
	case ClientMessage:
		if(((Atom)a.xclient.data.l[0]) == xlib.wmdeletewindow.a)
		{
			infoAboutWindow->bClose = 1;
			return;
		}
		break;
	case ConfigureNotify:
		infoAboutWindow->ifResizing.newWidthInPixels = a.xconfigure.width;
		infoAboutWindow->ifResizing.newHeightInPixels = a.xconfigure.height;

		// NOTE: there seems to be no guarantee whether ConfigureNotify..
		//       .. happens before/after FocusIn thus possible that..
		//       .. infoAboutWindow->bResize is set to 1 more than once for..
		//       .. one resize currently but I haven't seen worse than 2..
		//       .. times for one resize
		// NOTE: if pressed maximize ConfigureNotify always happened after..
		//       .. FocusIn for me
		//       if dragged a window border for resizing around very quickly..
		//       a ConfigureNotify sometimes arrived after FocusIn for me
		if(infoAboutWindow->bIsFocused == 1)
		{
			if(should_window_be_resized(infoAboutWindow) == 1)
			{
				infoAboutWindow->bResize = 1;
			}
		}
		break;
	case FocusIn:
		if(on_window_focused != NULL)
		{
			on_window_focused(window);
		}
		
		infoAboutWindow->bIsFocused = 1;

		// NOTE: for ConfigureNotify resize only happens if is focused thus..
		//       .. here if focused check whether should be resized because..
		//       .. of resize happened while wasn't focused?
		if(should_window_be_resized(infoAboutWindow) == 1)
		{
			infoAboutWindow->bResize = 1;
		}
		break;
	case FocusOut:
		infoAboutWindow->bUnfocus = 1;
		if(on_window_unfocused != NULL)
		{
			on_window_unfocused(window);
		}
	
		infoAboutWindow->bIsFocused = 0;
		break;
	}
}
#endif

#if defined(_WIN32)
static void poll_win32()
{
	MSG a;
	// "If hWnd is -1, PeekMessage retrieves only messages on the..
	// .. current thread's message queue whose hwnd value is NULL, that..
	// .. is, thread messages as posted by PostMessage (when the hWnd..
	// .. parameter is NULL) or PostThreadMessage.",
	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-peekmessagea
	// v
	// PostMessage/PostThreadMessage
	//            v	
	//HWND b = i == -1 ? -1 : infoAboutWindow->hwnd.a;
	while(PeekMessage(&a, NULL, 0, 0, PM_NOREMOVE) != 0)
	{
		// "GetMessage blocks until a message is posted before returning",..
		// .. https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage
		BOOL b = GetMessage(&a, NULL, 0, 0);
		
		int window = -1;
		// NOTE: ^
		//       possible that a.hwnd != NULL but window == -1 if the window..
		//       .. was closed from GUI?
		//       ^
		//       caused exit with -1073741819 which is C0000005 in 32-bit..
		//       .. hex which is STATUS_ACCESS_VIOLATION,..
		//       .. https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
		struct info_about_window_t* infoAboutWindow;
		if(a.hwnd != NULL)
		{
			for(int i = 0; i < numWindowsTheresRoomFor; ++i)
			{
				struct info_about_window_t* c = &infoPerWindow[i];
				
				if(c->source == NULL)
				{
					continue;
				}
				
				if(c->win32.hwnd.a == a.hwnd)
				{
					window = i;
					infoAboutWindow = &infoPerWindow[i];
					break;
				}
			}
		}
		
		if(b <= 0)
		{
			if(b == -1)
			{
				if(on_print != NULL)
				{
					int d;
					getlasterror_to_string(&d, NULL);
					char e[d];
					getlasterror_to_string(&d, e);
										
					on_printf(stdout, "warning: %s in %s\n", e, __FUNCTION__);
					// ^
					// warning here as assuming this doesn't persé mean..
					// .. program exit?
				}
				break;
				// ^
				// not sure if should break here?
			}
			
			// WM_QUIT
			
			if(on_print != NULL)
			{
				on_printf(stdout, "warning: WM_QUIT received which window-mini does not support in %s\n", __FUNCTION__);
			}
		}
		
		if(window != -1)
		{
			if(infoAboutWindow->bResize == 1)
			{
				if(on_window_resized != NULL)
				{
					on_window_resized(window, infoAboutWindow->ifResizing.newWidthInPixels, infoAboutWindow->ifResizing.newHeightInPixels);
				}
				infoAboutWindow->widthInPixels = infoAboutWindow->ifResizing.newWidthInPixels;
				infoAboutWindow->heightInPixels = infoAboutWindow->ifResizing.newHeightInPixels;
				infoAboutWindow->bResize = 0;
			}
		}

		TranslateMessage(&a);
		DispatchMessage(&a);
	}
}
#else //< #elif defined(__linux__)
static void poll_xlib()
{
	int a = XPending(xlib.display.a);
	while(a > 0)
	{
		for(int i = 0; i < a; ++i)
		{
			struct
			{
				XEvent a;
			} xevent;

			XNextEvent(xlib.display.a, &xevent.a);
			
			int window = -1;
			for(int j = 0; j < numWindowsTheresRoomFor; ++j)
			{
				if(infoPerWindow[j].source == NULL)
				{
					continue;
				}

				if(infoPerWindow[j].xlib.window.a == xevent.a.xany.window)
				{
					window = j;
					break;
				}
			}
			if(window == -1)
			{
				on_printf(stdout, "warning: xevent dropped as window == -1 in %s\n", __FUNCTION__);
				continue;
			}

			my_on_xevent(xevent.a);
			if(infoAboutWindow->bClose == 1)
			{
				if(on_window_closed != NULL)
				{
					if(on_window_closed(window) == 0)
					{
						// TODO: cancel closing window
					}
				}
				infoAboutWindow->source = NULL;
				
				infoAboutPoll.bWasAnyWindowClosed = 1;
				continue;
			}
			if(infoAboutWindow->bResize == 1)
			{
				on_window_resized(window, infoAboutWindow->ifResizing.newWidthInPixels, infoAboutWindow->ifResizing.newHeightInPixels);
				infoAboutWindow->widthInPixels = infoAboutWindow->ifResizing.newWidthInPixels;
				infoAboutWindow->heightInPixels = infoAboutWindow->ifResizing.newHeightInPixels;
				infoAboutWindow->bResize = 0;
			}
		}
		a = XPending(xlib.display.a);
	}
}
#endif
int wm_poll()
{
	if(bIsLoaded == 0)
	{
		return -1;
	}
	
	if(numWindows == 0)
	{
		return -1;
	}
	
	infoAboutPoll.bWasAnyWindowClosed = 0;

#if defined(_WIN32)
	poll_win32();
#else //< #elif defined(__linux__)
	poll_xlib();
#endif

	if(infoAboutPoll.bWasAnyWindowClosed == 1)
	{
		// NOTE: not required to call DestroyWindow/XDestroyWindow as window..
		//       .. is automatically done?
	
		int indexToLastWindow = -1;
		for(int i = numWindowsTheresRoomFor - 1; i >= 0; --i)
		{
			if(infoPerWindow[i].source != NULL)
			{
				indexToLastWindow = i;
				break;
			}
		}
		
		// if indexToLastWindow == -1.. (numWindowsTheresRoomFor - 1) - -1..
		// .. == numWindowsTheresRoomFor
		// v
		if(indexToLastWindow < numWindowsTheresRoomFor)
		{
			int lastNumElementsToRemove = (numWindowsTheresRoomFor - 1) - indexToLastWindow;
			if(lastNumElementsToRemove > 0)
			{
				remove_last_num_elements2(&numWindowsTheresRoomFor, &infoPerWindow, lastNumElementsToRemove);
			}
		}
		
		--numWindows;
	}
	
	return 1;
}
