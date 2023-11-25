#include "window_mini.hpp"


WMWindow::~WMWindow()
{
	if(window == -1)
	{
		return;
	}
	
	wm_remove_window(window);
}

bool WMWindow::edit(int conditions)
{
	if(window == -1)
	{
		return false;
	}
	
	update_source();
	if(wm_edit_window(window, conditions) == 0)
	{
		return false;
	}
	
	return true; //< consider -1 (no change) not an error
}

void WMWindow::update_source()
{
	/*
	source.minWidthInPixels = minWidthInPixels;
	source.minHeightInPixels = minHeightInPixels;
	source.maxWidthInPixels = maxWidthInPixels;
	source.maxHeightInPixels = maxHeightInPixels;
	source.widthInPixels = widthInPixels;
	source.heightInPixels = heightInPixels;
	source.title = title;
	source.bFullscreen = bFullscreen ? 1 : 0;
	*/
	bNotFullscreen = bFullscreen ? 1 : 0;
}

bool wm_add_window(struct wm_window_parameters_t& parameters, WMWindow* window)
{
	struct wm_add_window_parameters_t a;
	a.flags = parameters.flags;
	
	window->minWidthInPixels = parameters.minWidthInPixels;
	window->minHeightInPixels = parameters.minHeightInPixels;
	window->maxWidthInPixels = parameters.maxWidthInPixels;
	window->maxHeightInPixels = parameters.maxHeightInPixels;
	window->widthInPixels = parameters.widthInPixels;
	window->heightInPixels = parameters.heightInPixels;
	window->title = parameters.title;
	window->bFullscreen = parameters.bFullscreen;
	window->update_source();
	//if(wm_add_window(&a, &window->source, &window->window) != 1)
	if(wm_add_window(&a, window->get_source(), &window->window) != 1)
	{
		return false;
	}
	
	return true;
}
