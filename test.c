#include <window_mini.h>
#include <test_mini.h>
#include <clock_mini.h>


void on_print(char* a, FILE* b)
{
	fputs(a, b);
}

void popup(struct wm_info_about_window_t* infoAboutWindow, char* a)
{
#if defined(_WIN32)
	MessageBoxA(infoAboutWindow == NULL ? NULL : infoAboutWindow->win32.hwnd.a, a, "", MB_OK);
#else //< #elif defined(__linux__)
	printf("%s\n", a);
#endif
}

int test_1()
{
	int bSuccess = 1;
	
	wm_set_on_print(&on_print);

	do
	{
		int a = wm_load();
		if(a != 1)
		{
			fprintf(stderr, "error: wm_load() == %i\n", a);
			bSuccess = 0;
			break;
		}
		
		int b = wm_unload();
		if(b != 1)
		{
			fprintf(stderr, "error: wm_unload() == %i\n", b);
			bSuccess = 0;
			break;
		}
	} while(0);
	
	wm_unset_on_print();
	
	return bSuccess;
}

int test_2()
{
	int bSuccess = 1;
	
	wm_set_on_print(&on_print);
	
	do
	{
		int a = wm_load();
		if(a != 1)
		{
			fprintf(stderr, "error: wm_load() == %i\n", a);
			bSuccess = 0;
			break;
		}
		
		struct wm_add_window_parameters_t parameters = wm_add_window_parameters_default;
		struct wm_window_source_t source = wm_window_source_default;
		
		int window;
		int b = wm_add_window(&parameters, &source, &window);
		if(b != 1)
		{
			fprintf(stderr, "error: wm_add_window == %i\n", b);
			bSuccess = 0;
			break;
		}
		
		double c = cm_get_seconds();
		double d = c;
		while(d - c <= 0.5f)
		{
			d = cm_get_seconds();
			//printf("d - c is %f\n", d - c);
			int g = wm_poll();
			if(g != 1)
			{
				fprintf(stderr, "error: wm_poll() == %i\n", g);
				bSuccess = 0;
				break;
			}
		}
		// ^
		// poll for 0.25second
		if(bSuccess == 0)
		{
			break;
		}
		
		int e = wm_remove_window(window);
		if(e != 1)
		{
			fprintf(stderr, "error: wm_remove_window == %i\n", e);
			bSuccess = 0;
			break;
		}
		
		int f = wm_unload();
		if(f != 1)
		{
			fprintf(stderr, "error: wm_unload() == %i\n", f);
			bSuccess = 0;
			break;
		}
	} while(0);
	
	wm_unset_on_print();
	
	return bSuccess;
}

int test_3_success;

int test_3_window;

int test_3_on_window_closed_called;
int test_3_on_window_resized_called;
int test_3_on_window_focused_called;
int test_3_on_window_unfocused_called;

int on_window_closed(int window)
{
	test_3_on_window_closed_called = 1;
	
	if(window != test_3_window)
	{
		fprintf(stderr, "error: window != %i in on_window_closed (window == %i)\n", test_3_window, window);
		test_3_success = 0;
	}
	
	return 1;
}

void on_window_resized(int window, int widthInPixels, int heightInPixels)
{
	test_3_on_window_resized_called = 1;
	
	if(window != test_3_window)
	{
		fprintf(stderr, "error: window != %i in on_window_resized (window == %i)\n", test_3_window, window);
		test_3_success = 0;
	}
}

void on_window_focused(int window)
{
	test_3_on_window_focused_called = 1;
	
	if(window != test_3_window)
	{
		fprintf(stderr, "error: window != %i in on_window_focused (window == %i)\n", test_3_window, window);
		test_3_success = 0;
	}
}
void on_window_unfocused(int window)
{
	test_3_on_window_unfocused_called = 1;
	
	if(window != test_3_window)
	{
		fprintf(stderr, "error: window != %i in on_window_unfocused (window == %i)\n", test_3_window, window);
		test_3_success = 0;
	}
}

int test_3()
{
	test_3_success = 1;
	
	test_3_on_window_closed_called = 0;
	test_3_on_window_resized_called = 0;
	test_3_on_window_focused_called = 0;
	test_3_on_window_unfocused_called = 0;

	wm_set_on_print(&on_print);
	
	do
	{
		int a = wm_load();
		if(a != 1)
		{
			fprintf(stderr, "error: wm_load() == %i\n", a);
			test_3_success = 0;
			break;
		}
		
		struct wm_add_window_parameters_t parameters = wm_add_window_parameters_default;
		struct wm_window_source_t source = wm_window_source_default;
		
		source.widthInPixels = 150;
		source.heightInPixels = 150;
		
		int b = wm_add_window(&parameters, &source, &test_3_window);
		if(b != 1)
		{
			fprintf(stderr, "error: wm_add_window == %i\n", b);
			test_3_success = 0;
			break;
		}
		
		popup(NULL, "unfocus the window..\n");
		// ^
		// causes focus on popup and thus unfocus of window thus popup..
		// .. before wm_set_on_window_focused
		wm_set_on_window_unfocused(&on_window_unfocused);
		
		int bDone = 0;
		while(bDone == 0)
		{
			int bWasOnWindowFocusedCalledOne = test_3_on_window_focused_called;
			int bWasOnWindowUnfocusedCalledOne = test_3_on_window_unfocused_called;
			int bWasOnWindowResizedCalledOne = test_3_on_window_resized_called;
			int bWasOnWindowClosedCalledOne = test_3_on_window_closed_called;
		
			int e = wm_poll();
			if(e != 1)
			{
				fprintf(stderr, "error: wm_poll() == %i\n", e);
				test_3_success = 0;
				break;
			}
			if(test_3_success == 0)
			{
				break;
			}
			
			if((bWasOnWindowUnfocusedCalledOne == 0) & (test_3_on_window_unfocused_called == 1))
			{
				popup(NULL, "focus the window..");
				// ^
				// closing popup causes unfocus of popup and focus of window..
				// .. thus popup before wm_set_on_window_focused
				wm_set_on_window_focused(&on_window_focused);
			}
			if((bWasOnWindowFocusedCalledOne == 0) & (test_3_on_window_focused_called == 1))
			{
				wm_set_on_window_resized(&on_window_resized);
				popup(NULL, "resize the window..");
			}
			if((bWasOnWindowResizedCalledOne == 0) & (test_3_on_window_resized_called == 1))
			{
				wm_set_on_window_closed(&on_window_closed);
				popup(NULL, "close the window..");
			}
			if((bWasOnWindowClosedCalledOne == 0) & (test_3_on_window_closed_called == 1))
			{
				bDone = 1;
			}
		}
		if(test_3_success == 0)
		{
			break;
		}
		
		wm_unset_on_window_focused();
		wm_unset_on_window_unfocused();
		wm_unset_on_window_resized();
		wm_unset_on_window_closed();
		
		int d = wm_unload();
		if(d != 1)
		{
			fprintf(stderr, "error: wm_unload() == %i\n", d);
			test_3_success = 0;
			break;
		}
	} while(0);
	
	wm_unset_on_print();
	
	return test_3_success;
}

int main(int argc, char** argv)
{
	TM_TEST(1, 9)
	TM_TEST(2, 1)
	TM_TEST(3, 1)
	
	return 0;
}
