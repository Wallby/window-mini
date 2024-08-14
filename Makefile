include makefile_mini.mk


$(call mm_start_parameters_t,a)
$(call mm_start,a)

$(call mm_add_library_parameters_t,b)
b.filetypes:=EMMLibraryfiletype_Static
b.c:=window_mini.c
b.h:=window_mini.h
$(call mm_add_library,window-mini,b)

$(call mm_add_library_parameters_t,c)
c.filetypes:=EMMLibraryfiletype_Static
c.c:=window_mini.c
c.cpp:=window_mini.cpp
c.h:=window_mini.h
c.hpp:=window_mini.hpp
$(call mm_add_library,window-mini2,c)

$(call mm_stop_parameters_t,h)
$(call mm_stop,h)
