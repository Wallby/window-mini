#pragma once

extern "C"
{
#include "window_mini.h"
}


class WMWindow
{
public:
	WMWindow() = default;
	~WMWindow();
	
	// NOTE: conditions == 0 deduce conditions from what has changed
	bool edit(int conditions = EWMEditWindowIf_Always);
	
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
	
	operator bool()
	{
		return window != -1;
	}
	
private:
	friend bool wm_add_window(struct wm_window_parameters_t& parameters, WMWindow* window);
	
	struct wm_window_source_t* get_source()
	{
		// NOTE: should be ok according to https://stackoverflow.com/a/2007980
		return (struct wm_window_source_t*)&minWidthInPixels;
	}
	void update_source();
	
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
