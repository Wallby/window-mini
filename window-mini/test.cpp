#include <window_mini.hpp>
#include <clock_mini.h>
#include <test_mini.h>


void on_print(char* a, FILE* b)
{
	fputs(a, b);
}

// "Window" is already in use by xlib thus instead "MyWindow"
// v
class MyWindow : WMWindow
{
public:
	virtual bool on_closed() override
	{
		//...
		
		return true;
	}
	virtual void on_focused() override
	{
		//...
	}
	virtual void on_unfocused() override
	{
		//...
	}
	virtual void on_resized(int widthInPixels, int heightInPixels) override
	{
		//...
	
		WMWindow::on_resized(widthInPixels, heightInPixels);
	}
};

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
			break;
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

		int bSuccess = 1;

		double a = cm_get_seconds();
		double b = a;
		while(b - a < 0.5f)
		{
			int d = wm_poll();
			if(d != 1)
			{
				fprintf(stderr, "error: wm_poll() != 1 (wm_poll == %i)\n", d);
				bSuccess = 0;
				break;
			}

			b = cm_get_seconds();
		}
		if(bSuccess == 0)
		{
			break;
		}
				
		window.widthInPixels = 100;
		window.heightInPixels = 100;
		if(!window.edit())
		{
			fputs("error: !window.edit()\n", stderr);
			break;
		}

		int c = wm_poll();
		if(c != 1)
		{
			fprintf(stderr, "error: wm_poll() != 1 (wm_poll == %i)\n", c);
			break;
		}
		// NOTE: ^
		//       currently won't result in call of on_window_resized as..
		//       .. wm_edit_window is not implemented 

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
