include makefile_mini.mk


$(call mm_start_parameters_t,a)
a.ignoredbinaries:=^test[0-9]+$(MM_EXECUTABLE_EXTENSION)$$
$(call mm_start,a)

$(call mm_add_library_parameters_t,b)
b.filetypes:=EMMLibraryfiletype_Static
b.c:=window_mini.c
#b.h:=window_mini.h
$(call mm_add_library,window-mini,b)

$(call mm_add_library_parameters_t,c)
c.filetypes:=EMMLibraryfiletype_Static
c.c:=window_mini.c
c.cpp:=window_mini.cpp
#c.h:=window_mini.h
#c.hpp:=window_mini.hpp
$(call mm_add_library,window-mini2,c)

$(call mm_add_executable_parameters_t,d)
d.c:=test.c
#d.libraries:=window-mini test-mini clock-mini
d.libraries:=window-mini
d.lib:=gdi32 test-mini
d.libFolders:=../test-mini
d.hFolders:=../test-mini/ ../clock-mini
# ^
# to be able to use < and > in #include <window_mini.h>
d.gccOrG++:=-Wl,--wrap=malloc,--wrap=free,--wrap=main
$(call mm_add_executable,test1,d)

$(call mm_add_executable_parameters_t,e)
e.cpp:=test.cpp
e.libraries:=window-mini2
e.lib:=gdi32
$(call mm_add_executable,test2,e)

$(call mm_add_test_parameters_t,f)
f.executables:=test1
$(call mm_add_test,test1,f)

$(call mm_add_test_parameters_t,g)
g.executables:=test2
$(call mm_add_test,test2,g)

$(call mm_stop_parameters_t,h)
$(call mm_stop,h)
