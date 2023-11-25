#include "window_mini.hpp"


void on_print(char* a, FILE* b)
{
	fputs(a, b);
}

enum
{
	EProgress_WindowMiniLoaded = 1,
	EProgress_All
};
int main(int argc, char** argv)
{
	wm_set_on_print(&on_print);
	
	int progress = 0;
	
	do
	{
		if(wm_load() != 1)
		{
			fputs("error: wm_load() != 1\n", stderr);
			return 1;
		}
		progress = EProgress_WindowMiniLoaded;

		WMWindow window;
		
		if(window)
		{
			fputs("error: window\n", stderr);
			break;
		}
		
		struct wm_window_parameters_t windowParameters;
		if(!wm_add_window(windowParameters, &window))
		{
			fputs("error: !wm_add_window\n", stderr);
			break;
		}

		if(!window)
		{
			fputs("error: !window\n", stderr);
			break;
		}
		
		window.widthInPixels = 100;
		window.heightInPixels = 100;
		if(!window.edit())
		{
			fputs("error: !window.edit()\n", stderr);
			break;
		}
		
		progress = EProgress_All;
	} while(0);
	if(progress >= EProgress_WindowMiniLoaded)
	{
		wm_unload();
	}
	
	wm_unset_on_print();
	
	if(progress != EProgress_All)
	{
		return 1;
	}
	
	return 0;
}
