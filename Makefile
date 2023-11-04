include makefile_mini.mk


$(call mm_start_parameters_t,a)
a.ignoredbinaries:=^test$(MM_EXECUTABLE_EXTENSION)$$
$(call mm_start,a)

$(call mm_add_library_parameters_t,b)
b.filetypes:=EMMLibraryfiletype_Static
b.c:=window_mini.c
#b.h:=window_mini.h
$(call mm_add_library,window-mini,b)

$(call mm_add_executable_parameters_t,c)
c.c:=test.c
#c.libraries:=window-mini test-mini clock-mini
c.libraries:=window-mini
c.lib:=gdi32 test-mini
c.libFolders:=../test-mini
c.hFolders:=../test-mini/ ../clock-mini
# ^
# to be able to use < and > in #include <window_mini.h>
c.gcc:=-Wl,--wrap=malloc,--wrap=free,--wrap=main
$(call mm_add_executable,test,c)

$(call mm_add_test_parameters_t,d)
d.executables:=test
$(call mm_add_test,test,d)

$(call mm_stop_parameters_t,e)
$(call mm_stop,e)
