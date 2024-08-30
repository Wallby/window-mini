#pragma once

extern "C"
{
#define WINDOW_MINI_HPP
#include "window_mini.h"
}


bool wm_load2();
#define wm_load wm_load2

bool wm_unload2();
#define wm_unload wm_unload2

//*****************************************************************************

class WMWindow
{
public:
	WMWindow() = default;
	~WMWindow();
	
	// NOTE: conditions == 0 deduce conditions from what has changed
	bool edit(int conditions = EWMEditWindowIf_Always);
	
	bool close();
	
	// struct wm_source_t..
	int minWidthInPixels;
	int minHeightInPixels;
	int maxWidthInPixels;
	int maxHeightInPixels;
	int widthInPixels;
	int heightInPixels;
	char* title;
private:
	int bNotFullscreen;
public:
	// NOTE: sizeof(bool) not guaranteed to be == sizeof(int)
	bool bFullscreen;
	
	// struct wm_info_about_window_t..
#if defined(_WIN32)
	struct
	{
		struct
		{
			HWND a;
		} hwnd;
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
		struct
		{
			Window a;
		} window;
	} xlib;
#endif
	
	operator bool()
	{
		return window != -1;
	}
	
protected:
	virtual bool on_closed()
	{
		return true;
	}
	virtual void on_focused() {}
	virtual void on_unfocused() {}
	virtual void on_resized(int widthInPixels, int heightInPixels)
	{
		this->widthInPixels = widthInPixels;
		this->heightInPixels = heightInPixels;
	}

private:
	friend bool wm_add_window(struct wm_window_parameters_t& parameters, WMWindow* window);
	
	struct wm_window_source_t* get_source()
	{
		// NOTE: should be ok according to https://stackoverflow.com/a/2007980
		return (struct wm_window_source_t*)&minWidthInPixels;
	}
	void update_source();
	
	struct wm_info_about_window_t* get_info()
	{
#if defined(_WIN32)
		return (struct wm_info_about_window_t*)&win32;
#else
		return (struct wm_info_about_window_t*)&xlib;
#endif
	}
	void update_info();
	
	int window = -1;
	//struct wm_window_source_t source;
};

enum
{
	EWMWindowParametersFlag_Opengl = EWMAddWindowParametersFlag_Opengl
};

struct wm_window_parameters_t
{
	int flags = 0;
	int minWidthInPixels = -1;
	int minHeightInPixels = -1;
	int maxWidthInPixels = -1;
	int maxHeightInPixels = -1;
	int widthInPixels = -1;
	int heightInPixels = -1;
	char* title = NULL;
	bool bFullscreen = false;
};

bool wm_add_window(struct wm_window_parameters_t& parameters, WMWindow* window);
