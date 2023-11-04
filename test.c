#include <window_mini.h>
#include <test_mini.h>
#include <clock_mini.h>


void on_print(char* a, FILE* b)
{
	fputs(a, b);
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

int main(int argc, char** argv)
{
	TM_TEST(1, 9)
	TM_TEST2(2)
	
	return 0;
}
