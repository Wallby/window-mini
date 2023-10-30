include makefile_mini.mk


$(call mm_start_parameters_t,a)
a.ignoredbinaries:=^test$(MM_EXECUTABLE_EXTENSION)$$
$(call mm_start,a)

$(call mm_add_library_parameters_t,b)
b.c:=window_mini.c
#b.h:=window_mini.h
$(call mm_add_library,window-mini,b)

$(call mm_add_executable_parameters_t,c)
c.c:=test
#c.libraries:=window-mini
c.lib:=window-mini
c.hFolders:=./
# ^
# to be able to use < and > in #include <window_mini.h>
$(call mm_add_executable,test,c)

$(call mm_add_test_parameters_t,d)
d.executables:=test
$(call mm_add_test,test,d)

$(call mm_stop_parameters_t,e)
$(call mm_stop,e)
