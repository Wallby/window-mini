#define private public
#define protected public
#include "window_mini.hpp"


// NOTE: assumes..
//       .. numElementsToAppend > 0
void add_or_append_elements(int elementSize, int* numElements, void** elements, int numElementsToAppendOrAdd, void* elementsToAppendOrAdd)
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
	//memcpy(*elements + (elementSize * (*numElements)), elementsToAppendOrAdd, elementSize * numElementsToAppendOrAdd);
	memcpy(((char*)*elements) + (elementSize * (*numElements)), elementsToAppendOrAdd, elementSize * numElementsToAppendOrAdd);
	*numElements += numElementsToAppendOrAdd;
}
//#define add_or_append_elements2(numElements, elements, numElementsToAppendOrAdd, elementsToAppendOrAdd) add_or_append_elements(sizeof **elements, numElements, (void**)elements, numElementsToAppendOrAdd, (void*)elementsToAppendOrAdd)
#define add_or_append_element(elementSize, numElements, elements, elementToAppendOrAdd) add_or_append_elements(elementSize, numElements, elements, 1, elementToAppendOrAdd)
#define add_or_append_element2(numElements, elements, elementToAppendOrAdd) add_or_append_element(sizeof **elements, numElements, (void**)elements, (void*)elementToAppendOrAdd)

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

//*****************************************************************************

static int numWindowsTheresRoomFor = 0;
static int numWindows = 0;
static WMWindow** windows;

static void unregister_window(int window);
static int on_window_closed(int window)
{
	bool a = windows[window]->on_closed();
	
	unregister_window(window);
	
	windows[window]->window = -1;
	
	return a ? 1 : 0;
}
static void on_window_focused(int window)
{
	windows[window]->on_focused();
}
static void on_window_unfocused(int window)
{
	windows[window]->on_unfocused();
}
static void on_window_resized(int window, int widthInPixels, int heightInPixels)
{
	windows[window]->on_resized(widthInPixels, heightInPixels);
}

static void register_window(WMWindow* window)
{
	if(numWindows == 0)
	{
		wm_set_on_window_closed(&on_window_closed);
		wm_set_on_window_focused(&on_window_focused);
		wm_set_on_window_unfocused(&on_window_unfocused);
		wm_set_on_window_resized(&on_window_resized);
	}

	if(numWindows < numWindowsTheresRoomFor)
	{
		for(int i = 0; i < numWindowsTheresRoomFor; ++i)
		{
			if(windows[i] == NULL)
			{
				continue;
			}
			
			windows[i] = window;
			break;
		}
	}
	else
	{
		add_or_append_element2(&numWindowsTheresRoomFor, &windows, window);
	}
	
	++numWindowsTheresRoomFor;
}
// NOTE: possible to use "int window" here as using exact same algorithm for..
//       .. registering as is used in wm_add_window in window_mini.c
static void unregister_window(int window)
{
	if(numWindows == 1)
	{
		wm_unset_on_window_closed();
		wm_unset_on_window_focused();
		wm_unset_on_window_unfocused();
		wm_unset_on_window_resized();
	}

	int indexToNextWindow = -1;
	for(int i = window + 1; i < numWindowsTheresRoomFor; ++i)
	{
		if(windows[i] != NULL)
		{
			indexToNextWindow = i;
			break;
		}
	}
	if(indexToNextWindow == -1)
	{
		int lastNumElementsToRemove = numWindowsTheresRoomFor - window;
		remove_last_num_elements2(&numWindowsTheresRoomFor, &windows, lastNumElementsToRemove);
	}
	else
	{
		windows[window] = NULL;
	}
	
	--numWindows;
}

//*****************************************************************************

WMWindow::~WMWindow()
{
	if(window == -1)
	{
		return;
	}
	
	unregister_window(window);
	
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
	update_info();
	
	return true; //< consider -1 (no change) not an error
}

bool WMWindow::close()
{
	if(window == -1)
	{
		return false;
	}
	
	unregister_window(window);
	
	wm_remove_window(window);
	
	window = -1;
	
	return true;
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
void WMWindow::update_info()
{
	wm_get_info_about_window(window, get_info()); //< as is window always returns 1
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
	window->update_info();
	
	register_window(window);
	
	return true;
}
