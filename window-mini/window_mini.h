#ifndef WINDOW_MINI_H
#define WINDOW_MINI_H

#include <stdio.h>

#if defined(_WIN32)
#include <Windows.h>
#else //< #elif defined(__linux__)
#include <X11/Xlib.h>
#endif


struct wm_info_t
{
#if defined(_WIN32)
	struct
	{
		struct
		{
			HINSTANCE a;
		} hinstance;
	} win32;
#else //< #elif defined(__linux__)
	struct
	{
		struct
		{
			Display* a;
		} display;
	} xlib;
#endif
};

//*****************************************************************************

// void(*on_print)(char* a, FILE* b);
void wm_set_on_print(void(*a)(char*, FILE*));
void wm_unset_on_print();

// NOTE: returns 1 if succeeded
//       returns 0 if failed
//       returns -1 if no code was executed
int wm_load();
// NOTE: returns 1 if succeeded
//       returns -1 no code was executed
int wm_unload();

// NOTE: returns 1 if succeeded
//       returns -1 if no code was executed
int wm_get_info(struct wm_info_t* info);

// NOTE: returns 1 if succeeded
//       returns -1 if no code was executed
int wm_poll();

//*****************************************************************************

struct wm_window_source_t
{
	// NOTE: if windows.. if min<Width/Height>InPixels == -1.. uses value..
	//       .. from GetSystemMetrics with SM_CXMINTRACK/SM_CYMINTRACK
	int minWidthInPixels;
	int minHeightInPixels;
	// NOTE: if windows.. if max<Width/Height>InPixels == -1.. uses value..
	//       .. from GetSystemMetrics with SM_CXMAXTRACK/SM_CYMAXTRACK
	int maxWidthInPixels;
	int maxHeightInPixels;
	// NOTE: if windows.. if <width/height>InPixels == -1.. uses CW_USEDEFAULT
	int widthInPixels;
	int heightInPixels;
	char* title; //< if == NULL.. title == ""
	int bFullscreen;
};
static struct wm_window_source_t wm_window_source_default = { .minWidthInPixels = -1, .minHeightInPixels = -1, .maxWidthInPixels = -1, .maxHeightInPixels = -1, .widthInPixels = -1, .heightInPixels = -1, .title = NULL, .bFullscreen = 0 };

struct wm_info_about_window_t
{
#if defined(__WIN32)
	struct
	{
		struct
		{
			HWND a;
		} hwnd;
	} win32;
#else //< #elif defined(__linux__)
	struct
	{
		struct
		{
			Window a;
		} window;
	} xlib;
#endif
};

//*****************************************************************************

// NOTE: returns 1 to close the window
//       returns 0 to not close the window
//int(*on_window_closed)(int window);
void wm_set_on_window_closed(int(*a)(int));
void wm_unset_on_window_closed();
//void(*on_window_resized)(int window, int widthInPixels, int heightInPixels);
void wm_set_on_window_resized(void(*a)(int,int,int));
void wm_unset_on_window_resized();
//void(*on_window_focused)(int window);
void wm_set_on_window_focused(void(*a)(int));
void wm_unset_on_window_focused();
//void(*on_window_unfocused)(int window);
void wm_set_on_window_unfocused(void(*a)(int));
void wm_unset_on_window_unfocused();

enum
{
	EWMAddWindowParametersFlag_Opengl = 1
};

struct wm_add_window_parameters_t
{
	int flags; //< 0 or one or more of EWMAddWindowParametersFlag
};
static struct wm_add_window_parameters_t wm_add_window_parameters_default = { .flags = 0 };

// NOTE: returns 1 if succeeded
//       returns 0 if failed
//       returns -1 if no code was executed
int wm_add_window(struct wm_add_window_parameters_t* parameters, struct wm_window_source_t* source, int* window);

enum
{
	EWMEditWindowIf_MinWidthAndOrMinHeightIsDifferent = 1,
	EWMEditWindowIf_MaxWidthAndOrMaxHeightIsDifferent = 2,
	EWMEditWindowIf_WidthAndOrHeightIsDifferent = 4,
	EWMEditWindowIf_TitleIsDifferent = 8,
	EWMEditWindowIf_FullscreenWasChanged = 16
};
#define EWMEditWindowIf_Always \
	EWMEditWindowIf_MinWidthAndOrMinHeightIsDifferent | \
	EWMEditWindowIf_MaxWidthAndOrMaxHeightIsDifferent | \
	EWMEditWindowIf_WidthAndOrHeightIsDifferent | \
	EWMEditWindowIf_TitleIsDifferent | \
	EWMEditWindowIf_FullscreenWasChanged

// NOTE: returns 1 if succeeded
//       returns 0 if failed
//       returns -1 if no code was executed
int wm_edit_window(int window, int conditions);

// NOTE: returns 1 if succeeded
//       returns -1 if no code was executed
int wm_remove_window(int window);

// NOTE: returns 1 if succeeded
//       returns -1 if no code was executed
int wm_get_info_about_window(int window, struct wm_info_about_window_t* infoAboutWindow);

#endif
