# usage..
# make #< default target makes all not ignored binaries
# make test #< make all test(s) if any and run them
# make release
# # ^
# # 1. make all not ignored binaries + make test(s) if any
# # 2. run test(s) if any and only continue if all test(s) passed
# # 3. make any releasetype(s) specified
# make clean #< clean all binaryparts and binaries
#
# rules..
# .. for paths use / only never \, if required (only if required)..
#    .. makefile-mini will automatically replace / with \
#
# terminology..
# .. project <- this Makefile
# .. resource <- e.g. for mm_add_library a library is a resource
# .. binarypartsource <- e.g. .c for .o
# .. binarypart <- e.g. .o for .lib/.a/.exe/<no extension>
# .. binarysource <- e.g. .glsl for .spv
# .. binary <- e.g. for $(call mm_add_library,staticlibrarytest,<..>)..
#    .. libstaticlibrarytest.<lib/a> is a binary
# .. library <- static|shared
# .. executable <- <no extension>/.exe, optionally.. portable (.AppImage/.exe)
# .. installer <- <.deb/.snap>/.msi
#
# projectname.. e.g. for myproject/makefile_mini.mk included by..
# .. myproject/Makefile.. myproject
#
# resourcename..
# .. may not contain . (see "external project address")
# .. may contain / (\ will be replaced by / for consistency)*
# .. per resourcetype every resourcename must be unique**
# ^
# * e.g. test/test.spv.h..
#   char $(notdir $(<.name>))_spv_h[] = { <...> };
#   v
#   char test_spv_h[] = { <...> };
#   OR
#   char $(subst /,_,$(dir $(<.name>)))$(notdir $(<.name>))_spv_h[] = {..
#   .. <...> };
#   v
#   char test_test_spv_h[] = { <...> };
#   ^
#   .makefile-mini/test/test.spv.h would still result in the above output as..
#   .. .makefile-mini is ignored here
# ** resourcename is always specified by type (e.g...
#    .. <mm_add_library_parameters_t>.libraries)
#    ^
#    <mm_start_parameters_t>.ignoredbinaries and..
#    .. <mm_stop_parameters>.ifRelease.<ignoredbinaries/if*.ignoredbinaries>..
#    .. are specified as regular expression, resourcename is not allowed here
#    ^
#    exceptions..
#    .. shader and shaderlibrary as both use .spv and .spv.h
#
# external project address..
# .. <projectname>:<resourcename> <- resourcename may not contain .
# .. <projectname>:<filepath> <- filepath must contain .
# ^
# if * contains a . in <projectname>:*.. * is considered a filepath
# otherwise.. * is considered a resourcename
# both resourcename and filepath may contain one or more / (e.g...
# .. test:test/test)
# if both external project address and binarypartsource is allowed..
# .. <binarypartsource> <- may not contain :
# .. <projectname>:* <- must contain :
# leave <projectname> empty for current project (e.g. :test for resource test)
# ^
# only files that don't use . ever are..
# .. Makefile (shouldn't be included as .mk is for inclusion only)
# .. linux executable (shouldn't be included as is platform specific thus..
#    .. use resourcename)

# # NOTE: $(1) == * (non cli) #< make may parse argument (e.g. for use in..
# #       .. $(subst <..>))
# #       $(2) == * (cli) #< make does not parse argument (e.g. $@)
# mm_cli_*

#MM_SAFETY:=


#******************************************************************************

# NOTE: https://stackoverflow.com/a/47927343/4825512
MM_EMPTY:=
define MM_NEWLINE:=

$(MM_EMPTY)
endef
MM_COMMA:=,
MM_SPACE:=$(MM_EMPTY) $(MM_EMPTY)
MM_PERCENT:=%

ifndef OS #< linux
MM_OS:=linux

MM_FOLDER_SEPARATOR:=/
MM_STATICLIBRARY_EXTENSION:=.a
MM_SHAREDLIBRARY_EXTENSION:=.so
MM_EXECUTABLE_EXTENSION:=
MM_SCRIPT_EXTENSION:=
MM_PORTABLEEXECUTABLE_EXTENSION:=.AppImage
MM_RELEASEINSTALLER_EXTENSIONS:=.deb .snap


MM_CLI_DEV_NULL:=/dev/null
else ifeq ($(OS), Windows_NT) #< windows
MM_OS:=windows

MM_FOLDER_SEPARATOR:=\$(MM_EMPTY)
MM_STATICLIBRARY_EXTENSION:=.lib
MM_SHAREDLIBRARY_EXTENSION:=.dll
MM_EXECUTABLE_EXTENSION:=.exe
MM_SCRIPT_EXTENSION:=.bat
MM_PORTABLEEXECUTABLE_EXTENSION:=.exe
MM_RELEASEINSTALLER_EXTENSIONS:=.msi

MM_CLI_DEV_NULL:=NUL
else
$(error os not supported)
endif
MM_EXECUTABLE_EXTENSION_OR_DOT:=$(if $(MM_EXECUTABLE_EXTENSION),$(MM_EXECUTABLE_EXTENSION),.)

ifndef OS #< linux
mm_cli_mkdir=mkdir $(1)

mm_cli_rm=rm -f $(1)

# NOTE: $(2) == non cli (see windows version of mm_cli_zip)
mm_cli_zip=zip -r9 $(1) $(2)

# NOTE: $(1) == one or element(s) of format <pattern>/<replacement>
define mm_cli_not_sed=
$(eval mm_cli_not_sed_a:=$(subst ",\",$(1)))
$(eval mm_cli_not_sed_b:=$(firstword $(mm_cli_not_sed_a)))
$(eval mm_cli_not_sed_c:=$(wordlist 2,$(words $(mm_cli_not_sed_a))))
"s/$(mm_cli_not_sed_b)/g$(patsubst %,;s/%/g,$(mm_cli_not_sed_c))"
endef
# NOTE: $(1) == one or element(s) of format <pattern>/<replacement>
#       $(2) == input
define mm_cli_sed=
$(2) | sed -e $(call mm_cli_not_sed,$(1))
endef
# NOTE: ^
#       sed "s/<..>/g;s/<..>/g" <..> > <..>
# NOTE: ^
#       " in <pattern>/<replacement> is replaced with \"
# NOTE: $(1) == one or element(s) of format <pattern>/<replacement>
#       $(2) == input filename
mm_cli_sed2=sed $(call mm_cli_not_sed,$(1)) $(2)
# NOTE: $(1) == one or element(s) of format <pattern>/<replacement>
#       $(2) == input filename
#       $(3) == output filename
mm_cli_sed3=$(call mm_cli_sed2,$(1),$(2)) > $(3)
else #< windows
# NOTE: mkdir outputs "The syntax of the command is incorrect." if there is..
#       .. a trailing /
mm_cli_mkdir=if not exist $(1) mkdir $(1)

# NOTE: del outputs "Invalid switch" if any forward / is used"
mm_cli_rm=if exist $(1) del $(subst /,\,$(1))

# NOTE: $(2) == non cli
mm_cli_zip=powershell "Compress-Archive $(subst $(MM_SPACE),$(MM_COMMA),$(strip $(2))) $(1)"
# NOTE: ^
#       cannot find documentation on omitting -Command for powershell but..
#       .. seems to work
#       ^
#       https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_powershell_exe?view=powershell-5.1#-command
# NOTE: ^
#       "use commas to separate the paths",..
#       .. https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.archive/compress-archive?view=powershell-7.3#-path
# NOTE: ^
#       strip in $(strip $(2)) to avoid multiple consecutive commas

# NOTE: $(1) == one or more element(s) of format <pattern>/<replacement>
define mm_cli_not_sed=
$(eval mm_cli_not_sed_a:=$(subst ",`\",$(1)))
$(subst /,\"$(MM_COMMA)\",$(patsubst %,-Replace \"%\",$(mm_cli_not_sed_a)))
endef
# NOTE: $(1) == one or more element(s) of format <pattern>/<replacement>
#       $(2) == input
mm_cli_sed=powershell "\"$(2)\" $(call mm_cli_not_sed,$(1))"
# NOTE: ^
#       powershell "Get-Content <..> -Replace \"<..>\",\"<..>\" -Replace..
#       .. \"<..>\",\"<..>\" | Set-Content <..>"
# NOTE: ^
#       " in <pattern>/<replacement> is replaced with `"
# NOTE: $(1) == one or more element(s) of format <pattern>/<replacement>
#       $(2) == input filename
mm_cli_sed2=powershell "Get-Content $(2) $(call mm_cli_not_sed,$(1))"
# NOTE: $(1) == one or more element(s) of format <pattern>/<replacement>
#       $(2) == input filename
#       $(3) == output filename
mm_cli_sed3=powershell "Get-Content $(2) $(call mm_cli_not_sed,$(1)) | Set-Content $(3)"
endif

ifndef OS #< linux
# NOTE: $(1) == filename except extension (non cli)
#       $(2) == inputfile (cli)
#       $(3) == outputfile (cli)
define mm_cli_hfile_from_file=
$(eval mm_cli_hfile_from_file_a:=$(shell tr -c a-zA-Z0-9 _ $(1)))
$(eval mm_cli_hfile_from_file_b:=$(shell tr a-z A-Z $(mm_cli_hfile_from_file_a)))
echo #ifndef $(mm_hfile_from_file_b)_H`n#define $(mm_cli_hfile_from_file_b)_H`n`n`nchar $(mm_cli_hfile_from_file_a)_h[] = {\" > $(3)
od -An -v -td1 $(2) | tr -s " " | tr -d "\n" | sed -z -e "s/ /, /g;s/^, //;s/, $ //" >> $(3)
echo \"`n`n#endif`n\" >> $(3)
endef
# NOTE: ^
#       https://unix.stackexchange.com/a/758531
else #< windows
# NOTE: $(1) == filename except extension (non cli)
#       $(2) == inputfile (cli)
#       $(3) == outputfile (cli)
define mm_cli_hfile_from_file=
$(strip\
	$(eval mm_cli_hfile_from_file_a:=$(shell powershell.exe "\"$(1)\" -Replace \"[^a-zA-Z0-9]\",\"_\""))\
	$(eval mm_cli_hfile_from_file_b:=$(shell powershell.exe ""\"$(mm_cli_hfile_from_file_a)\".ToUpper()"))\
	powershell.exe "$$$$a=(Get-Content -Encoding Byte -Raw $(2) | Out-String).Replace(\"`r`n\",\", \").TrimEnd(\", \"); Set-Content -NoNewline \"#ifndef $(mm_cli_hfile_from_file_b)_H`n#define $(mm_cli_hfile_from_file_b)_H`n`n`nchar $(mm_cli_hfile_from_file_a)_h[] = { $$$$a };`n`n#endif`n\"" $(3)\
)
endef
endif

# NOTE: $(1) == a
#       $(2) == b
# NOTE: if a == b.. returns 1
#       otherwise.. returns 0
# NOTE: not the same as $(if $(filter $(1),$(2),,) as mm_equals also..
#       .. returns 1 if both $(1) and $(2) are both empty
define mm_equals=
$(eval mm_equals_bAreBothEmpty:=0)
$(if $(1),,$(if $(2),,$(eval mm_equals_bAreBothEmpty:=1)))
$(if $(filter 1,$(mm_equals_bAreBothEmpty)),\
	1,\
	$(if $(filter $(1),$(2)),1,0)\
)
endef

# NOTE: switch(<$1>)
#       {
#       case <$(word 1,$(2))>:
#         <$(word 1,$(3))>
#         break;
#       case <$(word 2,$(2)>:
#         <$(word 2,$(3))>
#         break;
#       //...
#       };
define mm_switch=
$(if $(filter 1,$(call mm_equals,$(1),$(firstword $(2)))),\
	$(firstword $(3)),\
	$(eval mm_switch_a:=$(words $(2)))\
	$(if $(filter 1,$(mm_switch_a)),,\
		$(eval mm_switch_b:=$(wordlist 2,$(mm_switch_a),$(2)))\
		$(eval mm_switch_c:=$(wordlist 2,$(mm_switch_a),$(3)))\
		$(call mm_switch,$(1),$(mm_switch_b),$(mm_switch_c))\
	)\
)
endef

# NOTE: $(1) == elements variablename
mm_add_or_append_one_element=$(eval $(1)+=$(1).$(words $($(1))))

# NOTE: $(1) == pattern(s)
#       $(2) == text
# NOTE: like $(filter <..>) but using regular expression pattern
define mm_filter_using_patterns=
$(strip\
	$(eval mm_filter_using_patterns_a:=$(firstword $(2)))\
	$(eval mm_filter_using_patterns_b:=$(wordlist 2,$(words $(2)),$(2)))\
	$(eval mm_filter_using_patterns_c:=$(firstword $(1)))\
	$(eval mm_filter_using_patterns_d:=$(wordlist 2,$(words $(1)),$(1)))\
	$(if $(OS),\
	$(shell powershell "\"$(mm_filter_using_patterns_a)\"$(patsubst %,$(MM_COMMA)\"%\",$(mm_filter_using_patterns_b)) | Select-String -Pattern \"$(mm_filter_using_patterns_c)\"$(patsubst %,$(MM_COMMA)\"%\",$(mm_filter_using_patterns_d))"),\
	$(shell (echo "$(mm_filter_using_patterns_a)"$(patsubst %,; echo "%",$(mm_filter_using_patterns_b))) | grep -i -E "$(mm_filter_using_patterns_c)$(addprefix |,$(mm_filter_using_patterns_d))")\
	)\
)
endef
# NOTE: powershell "\"<..>\",\"<..>\" | Select-String -Pattern \"<..>\",\"<..>\""
#                   ^
#       no ^ required as surrounding with "" seems to escape all characters..
#       .. between ""
#       cmd.exe escaping would be powershell """"<..>""","""<...>""" |..
#       .. Select-String -Pattern """<..>""","""<..>""""
#       ^
#       https://stackoverflow.com/a/15262019
#       ^
#       powershell seems to also escape quotes if preceeded by \
# NOTE: (echo "<..>"; echo "<..>") | grep -i -E "<..>|<..>"
#                                         ^
#                                -i for consistency between windows and linux?

# NOTE: $(1) == pattern(s)
#       $(2) == text
# NOTE: like $(filter-out <..>) but using regular expression pattern
mm_filter_out_using_patterns=$(filter-out $(call mm_filter_using_patterns,$(1),$(2)),$(2))

ifndef OS #< linux
# NOTE: list files in current folder and deeper folder(s) if any recursively
mm_get_path_to_file_per_file:=$(shell find -type f)
else #< windows
# NOTE: list files in current folder and deeper folder(s) if any recursively
mm_get_path_to_file_per_file:=$(subst ",,$(shell forfiles /s /c "cmd /c if @ISDIR==FALSE echo @RELPATH"))
# NOTE: ^
#       dir /S /B /A-D
#       ^
#       outputs absolute paths
endif

#******************************************************************************
#                                    checks
#******************************************************************************

# NOTE: $(1) == functionname
#       $(2) == variablename
mm_check_if_defined=$(if $(filter undefined,$(origin $(2))),$(error $(2) is not defined in $(1)),)

# NOTE: $(1) == functionname
#       $(2) == value pattern
#       $(3) == values variablename
define mm_check_if_valid_values=
$(eval mm_check_if_valid_values_a:=$(filter-out $(2),$($(3))))
$(if $(mm_check_if_valid_values_a),$(error $(3) contains invalid element(s) $(mm_check_if_valid_values_a) in $(1)),)
endef

# NOTE: $(1) == functionname
#       $(2) == value pattern
#       $(3) == value variablename
define mm_check_if_valid_value=
$(eval mm_check_if_valid_value_a:=$(filter-out $(2),$($(3))))
$(if $(mm_check_if_valid_value_a),$(error $(3) contains invalid value $(mm_check_if_valid_value_a) in $(1)),)
endef

#******************************************************************************
#                                    start
#******************************************************************************

$(shell $(call mm_cli_mkdir,.makefile-mini))

MM_PROJECTNAME:=$(lastword $(subst /, ,$(abspath .)))
# NOTE: .makefile-mini/<binarypart>
MM_BINARYPARTS:=
# NOTE: can contain both..
#       .. .makefile-mini/<ignoredbinary>
#       .. <notignoredbinary>
# NOTE: filepath not "path to file" as binary is not always there before..
#       .. this variable is used
MM_FILEPATH_PER_BINARY:=

# NOTE: MM_IGNOREDBINARIES_PATTERNS == <mm_start_parameters_t>.ignoredbinaries
MM_IGNOREDBINARIES_PATTERNS:=

# NOTE: .makefile-mini/<ignoredbinary>
MM_IGNOREDBINARIES:=
# NOTE: <notignoredbinary>
MM_NOTIGNOREDBINARIES:=

# NOTE: for sanity pattern will be surrounded by ^ and $
MM_SHAREDSHADER_PATTERN:=.*.spv
MM_STATICSHADER_PATTERN:=.*.spv.h
MM_SHADER_PATTERNS:=$(MM_SHAREDSHADER_PATTERN) $(MM_STATICSHADER_PATTERN)
MM_STATICLIBRARY_PATTERNS:=.*lib.*$(MM_STATICLIBRARY_EXTENSION)
MM_SHAREDLIBRARY_PATTERNS:=.*lib.*$(MM_SHAREDLIBRARY_EXTENSION)
ifndef OS #< linux
# NOTE: [^/\\\.\n] not required as filepath cannot contain newline
MM_EXECUTABLE_PATTERNS:=.*[/\\][^/\\\.]+ [^/\\\.]+
else
MM_EXECUTABLE_PATTERNS:=.*.exe
endif

# NOTE: $(1) == variablename
define mm_start_parameters_t=
$(eval $(1).ignoredbinaries:=)
endef
# NOTE: ^
#       .ignoredbinaries == empty or pattern(s), each pattern is a regular..
#       .. expression without any space(s) allowed)

#*********************************** checks ***********************************

# NOTE: $(1) == functionname
#       $(2) == <mm_start_parameters_t>
define mm_check_start_parameters_t=
$(call mm_check_if_defined,$(1),$(2).ignoredbinaries)
endef

#******************************************************************************

# NOTE: $(1) == <mm_start_parameters_t>
define mm_start=
$(if $(filter undefined,$(origin MM_SAFETY)),,\
	$(call mm_check_start_parameters_t,$(0),$(1))\
)
$(eval MM_IGNOREDBINARIES_PATTERNS:=$$($(1).ignoredbinaries))
endef
# NOTE: ^
#       $$ in $(eval MM_IGNOREDBINARIES_PATTERN:=$$<..>) because patterns..
#       .. may contain $ which should not be directly supplied to eval

# NOTE: $(1) == binary
# NOTE: assumes $(1) does not start with .makefile-mini/
mm_is_binary_ignored=$(if $(MM_IGNOREDBINARIES_PATTERNS),$(if $(call mm_filter_using_patterns,$(MM_IGNOREDBINARIES_PATTERNS),$(1)),1,0),0)

# NOTE: $(1) == binary
mm_get_binaryfilepath_from_binary=$(if $(filter 1,$(call mm_is_binary_ignored,$(1))),.makefile-mini/$(1),$(1))

# NOTE: $(1) == binary
#       $(2) == filepath variablename
# NOTE: assumes $(1) does not start with .makefile-mini/
define mm_add_binary=
$(eval mm_add_binary_filepath:=)
$(if $(filter 1,$(call mm_is_binary_ignored,$(1))),\
	$(eval mm_add_binary_filepath:=.makefile-mini/$(1))\
	$(eval MM_IGNOREDBINARIES+=$(1)),\
	$(eval mm_add_binary_filepath:=$(1))\
	$(eval MM_NOTIGNOREDBINARIES+=$(1))\
)
$(eval MM_FILEPATH_PER_BINARY+=$(mm_add_binary_filepath))
$(eval $(2):=$(mm_add_binary_filepath))
endef

# NOTE: $(1) == binarypart
# NOTE: assumes $(1) does not start with .makefile-mini/
mm_add_binarypart=$(eval MM_BINARYPARTS+=$(1))

#******************************************************************************
#                                   resources
#******************************************************************************

#define mm_info_about_resource_t=
#$(eval .name:=)
#endef

# NOTE: $(1) == RESOURCETYPE
#       $(2) == resourcename
# NOTE: if there is a resource for which <mm_info_about_resource_t>.name ==..
#       .. $(2).. returns 1
#       otherwise.. returns 0
define mm_is_resource=
$(eval mm_is_resource_bIsResource:=0)
$(foreach mm_is_resource_infoAboutResource,$(MM_INFO_PER_$(1)),\
	$(if $(filter $($(mm_is_resource_infoAboutResource).name),$(2)),\
		$(eval mm_is_resource_bIsResource:=1)\
	,)\
)
$(if $(filter 1,$(mm_is_resource_bIsResource)),1,0)
endef

#******************************************************************************
#                                     glsl
#******************************************************************************

# NOTE: $(1) == variablename
define mm_info_about_spvasm_from_glsl_t=
$(eval $(1).type:=)
$(eval $(1).spvasm:=)
$(eval $(1).glslangValidator:=)
endef
# NOTE: ^
#       .type == one of EMMShadertype
# NOTE: "It's a convention to name SPIR-V assembly and binary files with..
#       .. suffix .spvasm and .spv, respectively",
#       https://github.com/KhronosGroup/SPIRV-Tools#command-line-tools

MM_INFO_PER_SPVASM_FROM_GLSL:=

# NOTE: both .spvasm2 from .spvasm and .spv2 from .spvasm2 are temporary..
#       .. until possible to rename entrypoints using spirv-link,..
#       .. https://github.com/KhronosGroup/glslang/issues/605
define mm_info_about_spvasm2_and_spv2_from_spvasm_t=
$(eval $(1).a:=)
endef
# NOTE: ^
#       <a>.spvasm2
#       <a>.spv2

MM_INFO_PER_SPVASM2_AND_SPV2_FROM_SPVASM:=

#*********************************** shader ***********************************

EMMShadertype:=EMMShadertype_Vertex EMMShadertype_Pixel

# NOTE: for consistency with libraries a shader/shaderlibary is..
#       .. shared if it is loaded at runtime
#       .. static if it is for compiling into a library/executable
EMMShaderfiletype:=EMMShaderfiletype_Shared EMMShaderfiletype_Static
EMMShaderfiletype_All:=$(EMMShaderfiletype)

# NOTE: $(1) == variablename
define mm_add_shader_parameters_t=
$(eval $(1).filetypes:=$(EMMShaderfiletype_All))
$(eval $(1).type:=)
$(eval $(1).glsl:=)
$(eval $(1).glslangValidator:=)
endef
# NOTE: ^
#       .filetypes == one or more of EMMShaderfiletype, may not be empty
#       .glsl == <filepath><filename>.glsl, must contain one element
#       .glslangValidator == i.e. glslangValidator <.glslangValidator>, may..
#       .. be empty

# NOTE: $(1) == variablename
define mm_info_about_shader_t=
$(eval $(1).name:=)
$(eval $(1).type:=)
$(eval $(1).filetypes:=)
$(eval $(1).spvasm:=)
$(eval $(1).spvFilepath:=)
$(eval $(1).spvHFilepath:=)
endef
# NOTE: .name == output files are..
#                .. <.name>.spv
#                .. <.name>.spv.h -> char <$(1).spv>_spv = { <...> };

MM_INFO_PER_SHADER:=

#********************************** checks  ***********************************

# NOTE: $(1) == functionname
#       $(2) == <mm_add_shader_parameters_t>
define mm_check_add_shader_parameters_t=
$(call mm_check_if_defined,$(1),$(2).filetypes)
$(call mm_check_if_defined,$(1),$(2).type)
$(call mm_check_if_defined,$(1),$(2).glsl)
$(call mm_check_if_defined,$(1),$(2).glslangValidator)

$(if $($(2).filetypes),,$(error $(2).filetypes may not be empty in $(1)))
$(call mm_check_if_valid_values,$(1),$(EMMShaderfiletype_All),$(2).filetypes)

$(if $($(2).type),,$(error $(2).type may not be empty in $(1)))
$(call mm_check_if_valid_value,$(1),$(EMMShadertype),,$(2).type)

$(if $(filter-out 1,$(words $($(2).glsl))),$(error $(2).glsl must contain one element in $(1)),)
$(call mm_check_if_valid_value,$(1),%.glsl,$(2).glsl)
endef

#******************************************************************************

# NOTE: $(1) == shadername
mm_is_shader=$(call mm_is_resource,SHADER,$(1))

# NOTE: $(1) == shadername
#       $(2) == <mm_add_shader_parameters_t>
# NOTE: binary is $(1).spvasm for every shadertype thus shadername is shared..
#       .. between shadertypes
define mm_add_shader=
$(if $(filter undefined,$(origin MM_SAFETY)),,\
	$(if $(filter 1,$(call mm_is_shader,$(1))),$(error attempted to add shader $(1) more than once in $(0)),)\
	$(call mm_check_add_shader_parameters_t,$(0),$(2))\
)
$(call mm_add_or_append_one_element,MM_INFO_PER_SHADER)
$(eval mm_add_shader_infoAboutShader:=$(lastword $(MM_INFO_PER_SHADER)))
$(call mm_info_about_shader_t,$(mm_add_shader_infoAboutShader))
$(eval $(mm_add_shader_infoAboutShader).name:=$(1))
$(eval $(mm_add_shader_infoAboutShader).type:=$($(2).type))
$(eval $(mm_add_shader_infoAboutShader).filetypes:=$($(2).filetypes))
$(eval $(mm_add_shader_infoAboutShader).spvasm:=$(patsubst %.glsl,%.spvasm,$($(2).glsl)))
$(eval mm_add_shader_bIsSpvasmFromGlsl:=0)
$(foreach mm_add_shader_infoAboutSpvasmFromGlsl,$(MM_INFO_PER_SPVASM_FROM_GLSL),\
	$(if $(filter $($(mm_add_shader_infoAboutSpvasmFromGlsl).spvasm),$($(2).spvasm)),\
		$(if $(filter $($(mm_add_shader_infoAboutSpvasmFromGlsl).type),$($(2).type)),,\
			$(error $($(2).spvasm) required more than once but with different type value in $(0))\
		)\
		$(if $(filter 0,$(call mm_equals,$($(mm_add_vertexshader_infoAboutSpvasmFromGlsl).glslangValidator).$($(2).glslangValidator)))\
			$(error $($(2).spvasm) required more than once but with different glslangValidator value in $(0))\
		,)\
		$(eval mm_add_shader_bIsSpvasmFromGlsl:=1)\
	,)\
)
$(if $(filter 0,$(mm_add_shader_bIsSpvasmFromGlsl)),\
	$(call mm_add_or_append_one_element,MM_INFO_PER_SPVASM_FROM_GLSL)\
	$(eval mm_add_shader_infoAboutSpvasmFromGlsl:=$(lastword $(MM_INFO_PER_SPVASM_FROM_GLSL)))\
	$(eval $(mm_add_shader_infoAboutSpvasmFromGlsl).type:=$($(2).type))\
	$(eval $(mm_add_shader_infoAboutSpvasmFromGlsl).spvasm:=$($(mm_add_shader_infoAboutShader).spvasm))\
	$(eval $(mm_add_shader_infoAboutSpvasmFromGlsl).glslangValidator:=$($(2).glslangValidator))\
	$(call mm_add_binarypart,$($(mm_add_shader_infoAboutShader).spvasm))\
,)
$(if $(filter EMMShaderfiletype_Shared,$($(2).filetypes)),\
	$(call mm_add_binary,$(1).spv,$(mm_add_shader_infoAboutShader).spvFilepath),\
	$(call mm_add_binarypart,$(1).spv)\
	$(eval $(mm_add_shader_infoAboutShader).spvFilepath:=.makefile-mini/$(1).spv)\
)
$(if $(filter EMMShaderfiletype_Static,$($(2).filetypes)),\
	$(call mm_add_binary,$(1).spv.h,$(mm_add_shader_infoAboutShader).spvHFilepath)\
,)
endef
# TODO: ^
#       if not EMMShaderfiletype_Shared.. .makefile-mini/$(1).spv
#       otherwise.. $(1).spv

#******************************************************************************
#                                      c
#******************************************************************************

# NOTE: $(1) == variablename
define mm_info_about_o_from_c_t=
$(eval $(1).c:=)
$(eval $(1).hFolders:=)
$(eval $(1).gcc:=)
$(eval $(1).o:=)
endef
# NOTE: ^
#       .o == if windows.. $(basename <.c>).o
#             if linux..
#             .. if sharedlibrary.. $(basename <.c>).shared.o
#             .. otherwise.. $(basename <.c>).static.o

MM_INFO_PER_O_FROM_C:=

#********************************** checks ***********************************

# NOTE: $(1) == functionname
#       $(2) == lib
define mm_check_lib=
$(eval mm_check_lib_invalidLib:=)
$(foreach mm_check_lib_lib,$(2),\
	$(if $(findstring .,$(mm_check_lib_lib)),\
		$(eval mm_check_lib_invalidLib+=$(mm_check_lib_lib))\
	)\
)
$(if $(mm_check_lib_invalidLib),\
	$(error $(2).lib contains invalid value(s) $(mm_check_lib_invalidLib) in $(1))\
,)
endef

#******************************************************************************

# NOTE: $(1) == functionname
#       $(2) == .hFolders
#       $(3) == .gcc
#       $(4) == .o
define mm_add_o_from_c=
$(foreach mm_add_o_from_c_o,$(4),\
	$(if $(OS),\
		$(eval mm_add_o_from_c_c:=$(basename $(mm_add_o_from_c_o)).c),\
		$(eval mm_add_o_from_c_c:=$(basename $(basename $(mm_add_o_from_c_o))).c)\
	)\
	$(eval mm_add_o_from_c_bIsOFromC:=0)\
	$(foreach mm_add_o_from_c_infoAboutOFromC,$(MM_INFO_PER_O_FROM_C),\
		$(if $(filter $($(mm_add_o_from_c_infoAboutOFromC).o),$(mm_add_o_from_c_o)),\
			$(if $(filter 0,$(call mm_equals,$($(mm_add_o_from_c_infoAboutOFromC).hFolders),$(2))),\
				$(error $(mm_add_o_from_c_o) required more than once but with different hFolders value in $(1))\
			,)\
			$(if $(filter 0,$(call mm_equals,$($(mm_add_o_from_c_infoAboutOFromC).gcc),$(3))),\
				$(error $(mm_add_o_from_c_o) required more than once but with different gcc value in $(1))\
			,)\
			$(eval mm_add_o_from_c_bIsOFromC:=1)\
		,)\
	)\
	$(if $(filter 0,$(mm_add_o_from_c_bIsOFromC)),\
		$(call mm_add_or_append_one_element,MM_INFO_PER_O_FROM_C)\
		$(eval mm_add_o_from_c_infoAboutOFromC:=$(lastword $(MM_INFO_PER_O_FROM_C)))\
		$(call mm_info_about_o_from_c_t,$(mm_add_o_from_c_infoAboutOFromC))\
		$(eval $(mm_add_o_from_c_infoAboutOFromC).c:=$(mm_add_o_from_c_c))\
		$(eval $(mm_add_o_from_c_infoAboutOFromC).hFolders:=$(2))\
		$(eval $(mm_add_o_from_c_infoAboutOFromC).gcc:=$(3))\
		$(if $(OS),,\
			$(eval $(mm_add_o_from_c_infoAboutOFromC).gcc+=-fpic -visibility=hidden)\
		)\
		$(eval $(mm_add_o_from_c_infoAboutOFromC).o:=$(mm_add_o_from_c_o))\
		$(call mm_add_binarypart,$(mm_add_o_from_c_o))\
	,)\
)
endef

#********************************** library ***********************************

# NOTE: library and shaderlibrary are separate as every library can be built..
#       .. from the same files and every shaderlibrary can be built from the..
#       .. same files
EMMLibraryfiletype:=EMMLibraryfiletype_Static EMMLibraryfiletype_Shared
EMMLibraryfiletype_All:=$(EMMLibraryfiletype)

# NOTE: $(1) == <mm_add_library_parameters_t>
# NOTE: if .filetypes is empty.. .c and .gcc must be empty
define mm_add_library_parameters_t=
$(eval $(1).filetypes:=)
$(eval $(1).c:=)
$(eval $(1).h:=)
$(eval $(1).hFolders:=)
$(eval $(1).lib:=)
$(eval $(1).libFolders:=)
$(eval $(1).libraries:=)
$(eval $(1).cGcc:=)
$(eval $(1).gcc:=)
endef
# NOTE: ^
#       .filetypes == empty (i.e. .h only) or one or multiple of..
#                     .. EMMLibraryfiletype
#       .hFolders == folders, equivalent to -I for gcc
#       .libraries == libraryname(s) and/or external project address(es)..
#                     .. each to a library (i.e. <projectname>:<libraryname>)
#       .lib == lib as in the first lib in lib<...>.<lib/dll/a/so>,..
#       .. equivalent to -l for gcc
#       .libFolders == folders, equivalent to -L for gcc
#       .gcc == gcc <.gcc> <...>
# NOTE: header only library (empty .filetypes) is such that external project..
#       .. address (<projectname>:<libraryname>) to header only library is..
#       .. possible

# NOTE: $(1) == variablename
define mm_info_about_library_t=
$(eval $(1).name:=)
$(eval $(1).filetypes:=)
$(eval $(1).o:=)
$(eval $(1).staticO:=)
$(eval $(1).sharedO:=)
$(eval $(1).h:=)
$(eval $(1).lib:=)
$(eval $(1).libFolders:=)
$(eval $(1).libraries:=)
$(eval $(1).gcc:=)
$(if $(OS),\
	$(eval $(1).windows.libfilepath:=)\
	$(eval $(1).windows.dllfilepath:=),\
	$(eval $(1).linux.afilepath:=)\
	$(eval $(1).linux.sofilepath:=)\
)
endef
# NOTE: ^
#       .staticO == if windows.. <.o>
#                    if linux.. compiled w.o. -fpic -fvisibility=hidden
#       .sharedO == if windows.. <.o>
#                    if linux.. == compiled w. -fpic -fvisibility=hidden
#       .o == if windows.. .o from .c
#             if linux.. <.staticO> <.sharedO>
#       .windows.<lib/dll>filepath == filepath to <.lib/.dll> binary
#       .linux.<a/so>filepath == filepath to <.a/.so> binary

MM_INFO_PER_LIBRARY:=

MM_LIBRARIES:=

#*********************************** checks ***********************************

# NOTE: $(1) == functionname
#       $(2) == <mm_add_library_parameters_t>
define mm_check_add_library_parameters_t=
$(call mm_check_if_defined,$(1),$(2).filetypes)
$(call mm_check_if_defined,$(1),$(2).c)
$(call mm_check_if_defined,$(1),$(2).h)
$(call mm_check_if_defined,$(1),$(2).hFolders)
$(call mm_check_if_defined,$(1),$(2).lib)
$(call mm_check_if_defined,$(1),$(2).libFolders)
$(call mm_check_if_defined,$(1),$(2).libraries)
$(call mm_check_if_defined,$(1),$(2).cGcc)
$(call mm_check_if_defined,$(1),$(2).gcc)

$(if $($(2).filetypes),
	$(call mm_check_if_valid_values,$(1),$(EMMLibraryfiletype_All),$(2).filetypes)\
	$(if $($(2).c),,$(error if $(2).filetypes is not empty.. $(2).c may not be empty in $(1)))\
	$(call mm_check_if_valid_values,$(1),%.c,$(2).c),\
	$(if $($(2).c),$(error if $(2).filetypes is empty.. $(2).c must be empty in $(1)),)\
	$(if $($(2).h),,$(error if $(2).filetypes is empty.. $(2).h may not be empty in $(1)))\
	$(if $($(2).cGcc),$(error if $(2).filetypes is empty.. $(2).cGcc must be empty in $(1)),)\
	$(if $($(2).gcc),$(error if $(2).filetypes is empty.. $(2).gcc must be empty in $(1)),)\
)
$(call mm_check_if_valid_values,$(1),%.h,$(2).h)
$(call mm_check_lib,$(1),$(2).lib)
endef
# TODO: ^
#       implement .h, .lib, .libFolders, .windows.*, .linux.*

#******************************************************************************

# NOTE: $(1) == libraryname
mm_is_library=$(call mm_is_resource,LIBRARY,$(1))

# NOTE: $(1) == libraryname
#       $(2) == <mm_add_library_parameters_t>
define mm_add_library=
$(if $(filter undefined,$(origin MM_SAFETY)),,\
	$(if $(filter 1,$(call mm_is_library,$(1))),$(error attempted to add library $(1) more than once in $(0)),)\
	$(call mm_check_add_library_parameters_t,$(0),$(2))\
)
$(eval MM_INFO_PER_LIBRARY+=MM_INFO_PER_LIBRARY.$(words $(MM_INFO_PER_LIBRARY)))
$(eval mm_add_library_infoAboutLibrary:=$(lastword $(MM_INFO_PER_LIBRARY)))
$(call mm_info_about_library_t,$(mm_add_library_infoAboutLibrary))
$(eval $(mm_add_library_infoAboutLibrary).name:=$(1))
$(eval $(mm_add_library_infoAboutLibrary).filetypes:=$($(2).filetypes))
$(if $(OS),\
	$(eval $(mm_add_library_infoAboutLibrary).o:=$(addsuffix .o,$(basename $($(2).c))))\
	$(eval $(mm_add_library_infoAboutLibrary).staticO:=$($(mm_add_library_infoAboutLibrary).o))
	$(eval $(mm_add_library_infoAboutLibrary).sharedO:=$($(mm_add_library_infoAboutLibrary).o)),\
	$(eval $(mm_add_library_infoAboutLibrary).o:=)\
	$(if $(filter EMMLibraryfiletype_Static,$($(mm_add_library_infoAboutLibrary).filetypes)),\
		$(eval $(mm_add_library_infoAboutLibrary).staticO:=$(addsuffix .static.o,$(basename $($(2).c))))\
		$(eval $(mm_add_library_infoAboutLibrary).o+=$($(mm_add_library_infoAboutLibrary).staticO))\
	,)\
	$(if $(filter EMMLibraryfiletype_Shared,$($(mm_add_library_infoAboutLibrary).filetypes)),\
		$(eval $(mm_add_library_infoAboutLibrary).sharedO:=$(addsuffix .shared.o,$(basename $($(2).c))))\
		$(eval $(mm_add_library_infoAboutLibrary).o+=$($(mm_add_library_infoAboutLibrary).sharedO))\
	,)\
)
$(call mm_add_o_from_c,$(0),$($(2).hFolders),$($(2).cGcc),$($(mm_add_library_infoAboutLibrary).o))
$(if $(filter EMMLibraryfiletype_Static,$($(2).filetypes)),\
	$(call mm_add_binary,lib$(1)$(MM_STATICLIBRARY_EXTENSION),$(mm_add_library_infoAboutLibrary).$(MM_OS)$(MM_STATICLIBRARY_EXTENSION)filepath)\
,)
$(if $(filter EMMLibraryfiletype_Shared,$($(2).filetypes)),\
	$(call mm_add_binary,lib$(1)$(MM_SHAREDLIBRARY_EXTENSION),$(mm_add_library_infoAboutLibrary).$(MM_OS)$(MM_SHAREDLIBRARY_EXTENSION)filepath)\
,)
$(eval MM_LIBRARIES+=$(1))
endef

#********************************* executable *********************************

# NOTE: default executablefiletype is..
#       .. if windows.. .exe
#       .. if linux.. <empty>
# NOTE: additionalexecutablefiletypes..
#       .. portable -> .exe/.AppImage
EMMAdditionalexecutablefiletype:=EMMAdditionalexecutablefiletype_Portable
EMMAdditionalexecutablefiletype_All:=$(EMMAdditionalexecutablefiletype)

define mm_add_executable_parameters_t
$(eval $(1).additionalfiletypes:=)
$(eval $(1).c:=)
$(eval $(1).hFolders:=)
$(eval $(1).lib:=)
$(eval $(1).libFolders:=)
$(eval $(1).libraries:=)
$(eval $(1).staticlibraries:=)
$(eval $(1).sharedlibraries:=)
$(eval $(1).cGcc:=)
$(eval $(1).gcc:=)
endef

define mm_info_about_executable_t=
$(eval $(1).name:=)
$(eval $(1).additionalfiletypes:=)
$(eval $(1).o:=)
$(eval $(1).lib:=)
$(eval $(1).libFolders:=)
$(eval $(1).libraries:=)
$(eval $(1).staticlibraries:=)
$(eval $(1).sharedlibraries:=)
$(eval $(1).gcc:=)
$(if $(OS),\
	$(eval $(1).windows.exefilepath:=),\
	$(eval $(1).linux.filepath:=)\
	$(eval $(1).linux.AppImagefilepath:=)
)
endef
# NOTE: ^
#       .linux.filepath == filepath to executable without extension (<no..
#       .. extension>)
#       .linux.AppImagefilepath == exception to "appimagefilepath" for..
#       .. consistency

MM_INFO_PER_EXECUTABLE:=

#*********************************** checks ***********************************

# NOTE: $(1) == functionname
#       $(2) == resourcetype plural
#       $(3) == RESOURCETYPE plural
#       $(4) == resources variablename
define mm_check_resources=
$(eval mm_check_resources_a:=$(filter-out $(MM_$(3)),$($(4))))
$(if $(mm_check_resources_a),$(error $(3) contains element(s) that aren't $(2) in $(1)) 
endef

# NOTE: $(1) == functionname
#       $(2) == libraries variablename
mm_check_libraries=$(call mm_check_resources,$(1),libraries,LIBRARIES,$(2))

# NOTE: $(1) == functionname
#       $(2) == <mm_add_executable_parameters_t>
define mm_check_add_executable_parameters_t=
$(call mm_check_if_defined,$(1),$(2).additionalfiletypes)
$(call mm_check_if_defined,$(1),$(2).c)
$(call mm_check_if_defined,$(1),$(2).hFolders)
$(call mm_check_if_defined,$(1),$(2).lib)
$(call mm_check_if_defined,$(1),$(2).libFolders)
$(call mm_check_if_defined,$(1),$(2).libraries)
$(call mm_check_if_defined,$(1),$(2).staticlibraries)
$(call mm_check_if_defined,$(1),$(2).sharedlibraries)
$(call mm_check_if_defined,$(1),$(2).cGcc)
$(call mm_check_if_defined,$(1),$(2).gcc)

$(call mm_check_if_valid_values,$(1),$(EMMAdditionalexecutablefiletypes_All),$(2).additionalfiletypes)
$(call mm_check_if_valid_values,$(1),%.c,$(2).c)
$(call mm_check_lib,$(1),$(2).lib)
$(call mm_check_libraries,$(1),$(2).libraries)
$(call mm_check_staticlibraries,$(1),$(2).staticlibraries)
$(call mm_check_sharedlibraries,$(1),$(2).sharedlib
endef

#******************************************************************************

# NOTE: $(1) == executablename
mm_is_executable=$(call mm_is_resource,EXECUTABLE,$(1))

# NOTE: $(1) == executablename
#       $(2) == <mm_add_executable_parameters_t>
define mm_add_executable=
$(if $(filter undefined,$(origin MM_SAFETY)),,\
	$(if $(filter 1,$(call mm_is_executable,$(1))),$(error attempted to add executable $(1) more than once in $(0)),)\
	$(call mm_check_add_executable_parameters_t,$(0),$(2))\
)
$(eval MM_INFO_PER_EXECUTABLE+=MM_INFO_PER_EXECUTABLE.$(words $(MM_INFO_PER_EXECUTABLE)))
$(eval mm_add_executable_infoAboutExecutable:=$(lastword $(MM_INFO_PER_EXECUTABLE)))
$(eval $(mm_add_executable_infoAboutExecutable).name:=$(1))
$(eval $(mm_add_executable_infoAboutExecutable).o:=$(patsubst %.c,%.o,$($(2).c)))
$(eval $(mm_add_executable_infoAboutExecutable).libraries:=$(filter $($(2).libraries),$(MM_LIBRARIES)))
$(eval $(mm_add_executable_infoAboutExecutable).staticlibraries:=$(filter $($(2).staticlibraries),$(MM_STATICLIBARIES)))
$(eval $(mm_add_executable_infoAboutExecutable).sharedlibraries:=$(filter $($(2).sharedlibraries),$(MM_SHAREDLIBRARIES)))
$(eval mm_add_executable_a:=$($(mm_add_executable_infoAboutExecutable).libraries) $($(mm_add_executable_infoAboutExecutable).staticlibraries) $($(mm_add_executable_infoAboutExecutable).sharedlibraries))
$(eval $(mm_add_executable_infoAboutExecutable).lib:=$($(2).lib) $(mm_add_executable_a))
$(eval $(mm_add_executable_infoAboutExecutable).libFolders:=$($(2).libFolders) $(if $(mm_add_executable_a),./,))
$(eval $(mm_add_executable_infoAboutExecutable).gcc:=$($(2).gcc))
$(call mm_add_o_from_c,$(0),$($(2).hFolders),$($(2).cGcc),$($(mm_add_executable_infoAboutExecutable).o))
$(call mm_add_binary,$(1)$(MM_EXECUTABLE_EXTENSION),$(mm_add_executable_infoAboutExecutable).$(MM_OS)$(MM_EXECUTABLE_EXTENSION_OR_DOT)filepath)
$(if $(OS),,\
	$(if $(filter EMMAdditionalexecutablefiletypes_Portable,$($(2).additionalfiletypes)),\
		$(call mm_add_binary,$(1).AppImage,$(2).linux.AppImagefilepath)\
	,)\
)
endef
# NOTE: ^
#       "$(if $(mm_add_executable_a),./,)" for libFolders because if any..
#       .. library in this folder.. -L./ is required for gcc

#******************************************************************************
#                                    tests
#******************************************************************************

# NOTE: $(1) == variablename
define mm_add_test_parameters_t=
$(eval $(1).executables:=)
$(eval $(1).scripts:=)
endef
# NOTE: ^
#       .scripts == if windows.. <script>.bat
#                   if linux.. <script>
# NOTE: "An executable file starting with an interpreter directive is [...]..
#       .. called a script",..
#       .. https://en.wikipedia.org/wiki/Shebang_(Unix)#Etymology
# NOTE: "A batch file is a script file",..
#       .. https://en.wikipedia.org/wiki/Batch_file
# NOTE: all executables and scripts for a test are started in parallel and..
#       .. the test is only done once all executables and scripts have..
#       .. exited (or if any executable/script fails)

# NOTE: $(1) == variablename
define mm_info_about_test_t=
$(eval $(1).name:=)
$(eval $(1).filepathPerExecutable:=)
$(eval $(1).scripts:=)
endef

MM_INFO_PER_TEST:=

MM_FILEPATH_PER_TESTBINARY:=
# NOTE: ^
#       currently only if windows.. .exe/ if linux.. <no extension>
#       ^
#       i.e. additional executable types cannot be tests

#*********************************** checks ***********************************

# NOTE: $(1) == functionname
#       $(2) == <mm_add_test_parameters_t>
define mm_check_add_test_parameters_t=
$(call mm_check_if_defined,$(1),$(2).executables)
$(call mm_check_if_defined,$(1),$(2).scripts)
endef

#******************************************************************************

# NOTE: $(1) == testname
mm_is_test=$(call mm_is_resource,TEST,$(1))

# NOTE: $(1) == testname
#       $(2) == <mm_add_test_parameters_t>
define mm_add_test=
$(if $(filter undefined,$(origin MM_SAFETY)),,\
	$(if $(filter 1,$(call mm_is_test,$(1))),$(error attempted to add test $(1) more than once in $(0)),)\
	$(call mm_check_add_test_parameters_t,$(0),$(2))\
)
$(eval MM_INFO_PER_TEST+=MM_INFO_PER_TEST.$(words $(MM_INFO_PER_TEST)))
$(eval mm_add_test_infoAboutTest:=$(lastword $(MM_INFO_PER_TEST)))
$(eval $(mm_add_test_infoAboutTest).name:=$(1))
$(foreach mm_add_test_executable,$($(2).executables),\
	$(foreach mm_add_test_infoAboutExecutable,$(MM_INFO_PER_EXECUTABLE),\
		$(if $(filter $(mm_add_test_executable),$($(mm_add_test_infoAboutExecutable).name)),\
			$(eval $(mm_add_test_infoAboutTest).filepathPerExecutable+=$($(mm_add_test_infoAboutExecutable).$(MM_OS)$(MM_EXECUTABLE_EXTENSION_OR_DOT)filepath))\
		,)\
	)\
)
$(eval $(mm_add_test_infoAboutTest).scripts:=$($(2).scripts))
$(if $($(2).executables),$(eval MM_FILEPATH_PER_TESTBINARY+=$($(mm_add_test_infoAboutTest).filepathPerExecutable)),)
endef

#******************************************************************************
#                                     stop
#******************************************************************************

MM_RELEASE:=

MM_RELEASEBINARIES:=
MM_RELEASEZIPBINARIES:=
MM_RELEASEINSTALLERBINARIES:=

MM_RELEASEFILES:=
MM_RELEASEZIPFILES:=
MM_RELEASEINSTALLERFILES:=

# NOTE: EMMReleasetype_Zip -> .<windows/linux>.zip
#       EMMReleasetype_Installer -> if windows.. .msi
#                                   if linux.. .deb and .snap
# NOTE: make release <(zip|installer)>
# NOTE: no "EMMReleasetype_Source" as source release == git main branch + tag
EMMReleasetype:=EMMReleasetype_Zip EMMReleasetype_Installer
EMMReleasetype_All:=$(EMMReleasetype)

# NOTE: $(1) == variablename
define mm_stop_parameters_t=
$(eval $(1).releasetypes:=)
$(eval $(1).ifRelease.additionalfiles:=)
$(eval $(1).ifRelease.ignoredbinaries:=)
$(eval $(1).ifRelease.ifZip.additionalfiles:=)
$(eval $(1).ifRelease.ifZip.ignoredbinaries:=)
$(eval $(1).ifRelease.ifInstaller.additionalfiles:=)
$(eval $(1).ifRelease.ifInstaller.ignoredbinaries:=)
endef
# NOTE: ^
#       .ifRelease.additionalfiles == empty or file(s) not made by..
#       .. makefile-mini to include in all releases
#       .ifRelease.ignoredbinaries == empty or binary/binaries to not..
#       .. include in all releases
#       .ifRelease.if*.additionalfiles == empty or file(s) not made by..
#       .. makefile-mini to include in corresponding release
#       .ifRelease.if*.ignoredbinaries == empty or binary/binaries to..
#       .. include in corresponding release

#*********************************** checks ***********************************

# NOTE: $(1) == functionname
#       $(2) == <mm_stop_parameters_t>
define mm_check_stop_parameters_t=
$(call mm_check_if_defined,$(1),$(2).releasetypes)
$(call mm_check_if_defined,$(1),$(2).ifRelease.additionalfiles)
$(call mm_check_if_defined,$(1),$(2).ifRelease.ignoredbinaries)
$(call mm_check_if_defined,$(1),$(2).ifRelease.ifZip.additionalfiles)
$(call mm_check_if_defined,$(1),$(2).ifRelease.ifZip.ignoredbinaries)
$(call mm_check_if_defined,$(1),$(2).ifRelease.ifInstaller.additionalfiles)
$(call mm_check_if_defined,$(1),$(2).ifRelease.ifInstaller,ignoredbinaries)

$(call mm_check_if_valid_values,$(1),$(EMMReleasetype_All),$(2).releasetypes)
endef

#******************************************************************************

# NOTE: $(1) == infoAboutSpvasmFromGlsl
define mm_add_spvasm_from_glsl_target=
$(eval mm_add_spvasm_from_glsl_target_a:=$(strip $(call mm_switch,$($(1).type),EMMShadertype_Vertex EMMShadertype_Pixel,vert frag)))
.makefile-mini/$($(1).spvasm):.makefile-mini/%.spvasm:%.glsl
	glslangValidator $($(1).glslangValidator) --quiet -o $(MM_CLI_DEV_NULL) --spirv-dis -V -S $(mm_add_spvasm_from_glsl_target_a) $$< > $$@
endef
# NOTE: ^
#       -o $(MM_CLI_DEV_NULL) is to work around glslangValidator always..
#       .. outputting a file, ..
#       .. https://github.com/KhronosGroup/glslang/issues/3368
#       --quiet as otherwise first line glslangValidator outputs is path to..
#       .. inputfile

# NOTE: $(1) == infoAboutShader
define mm_add_spv_from_spvasm_target=
$($(1).spvFilepath): .makefile-mini/$($(1).spvasm)
	spirv-as -o $$@ $$<
endef

# NOTE: $(1) == infoAboutShader
define mm_add_spv_h_from_spv_target=
$($(1).spvHFilepath): $($(1).spvFilepath)
	$(call mm_cli_hfile_from_file,$($(1).name)_spv,$$<,$$@)
endef

# NOTE: $(1) == infoAboutShader
define mm_add_shader_target=
$(call mm_add_spv_from_spvasm_target,$(1))
$(if $($(1).spvHFilepath),$(call mm_add_spv_h_from_spv_target,$(1)),)
endef
# NOTE: ^
#       $(1).spvFilepath is guaranteed to be not empty

# NOTE: $(1) == infoAboutOFromC
define mm_add_o_from_c_target=
$(if $(OS),.makefile-mini/$($(1).o):.makefile-mini/%.o:%.c,.makefile-mini/$($(1).o):$($(1).c))
	gcc $($(1).gcc) -o $$@ -c $$< $(addprefix -I,$($(1).hFolders))
endef

# NOTE: $(1) == infoAboutLibrary
define mm_add_staticlibrary_target=
$($(1).$(MM_OS)$(MM_STATICLIBRARY_EXTENSION)filepath): $(addprefix .makefile-mini/,$($(1).staticO))
	ar rcs $$@ $$^
endef

# NOTE: $(1) == infoAboutLibrary
define mm_add_sharedlibrary_target=
$($(1).$(MM_OS)$(MM_SHAREDLIBRARY_EXTENSION)filepath): $(addprefix .makefile-mini/,$($(1).sharedO))
	gcc -shared -o $$@ $$^
endef

# NOTE: $(1) == infoAboutLibrary
define mm_add_library_targets=
$(if $(filter EMMLibraryfiletype_Static,$($(1).filetypes)),$(call mm_add_staticlibrary_target,$(1)),)
$(if $(filter EMMLibraryfiletype_Shared,$($(1).filetypes)),$(call mm_add_sharedlibrary_target,$(1)),)	
endef

ifndef OS #< linux
# NOTE: $(1) == infoAboutExecutable
define mm_add_appimage_target=
endef
endif

# NOTE: $(1) == infoAboutExecutable
define mm_add_executable_targets=
$($(1).$(MM_OS)$(MM_EXECUTABLE_EXTENSION_OR_DOT)filepath): $(addprefix .makefile-mini/,$($(1).o))
	gcc $($(1).gcc) -o $$@ $$^ $(addprefix -L,$($(1).libFolders)) $(addprefix -l,$($(1).lib))
endef
# TODO: ^
#       .h prerequisites should be order only such that $$^ here still  works

#$(if $(OS),,$\
#$(if $($(1).linux.appimagefilepath),$(call mm_add_appimage_target))$\
#)

define mm_add_default_target=
.PHONY: default
default: $(MM_NOTIGNOREDBINARIES)
endef

define mm_add_test_target=
.PHONY: test
test: $(MM_FILEPATH_PER_TESTBINARY)
	$(foreach mm_add_test_target_infoAboutTest,$(MM_INFO_PER_TEST),$\
	$(foreach mm_add_test_target_executablefilepath,$($(mm_add_test_target_infoAboutTest).filepathPerExecutable),$\
	$(MM_NEWLINE)	$(if $(findstring /,$(mm_add_test_target_executablefilepath)),,.$(MM_FOLDER_SEPARATOR))$(mm_add_test_target_executablefilepath)$\
	)$\
	$(foreach mm_add_test_target_script,$($(mm_add_test_target_infoAboutTest).scripts),$\
	$(MM_NEWLINE)	$(if $(findstring /,$(mm_add_test_target_executablefilepath)),,.$(MM_FOLDER_SEPARATOR))$(mm_add_test_target_script)$(MM_SCRIPT_EXTENSION)"$\
	)$\
	)
endef
# NOTE: ^
#       I tried..
#       cmd /c "start /b $(mm_add_test_target_script).bat"
#       .. which caused powershell to keep cmd.exe open after make returned..
#       .. thus having manually call exit,..
#       .. https://stackoverflow.com/a/41411671
#       Start-Job -ScriptBlock { Set-Location $using:pwd; Invoke-Expression..
#       .. .\$args } -ArgumentList "$(mm_add_test_target_script).bat"
#       .. was the closest to working solution I found, but it broke the..
#       .. timeout command that I used to test whether scripts where ran in..
#       .. parallel, https://stackoverflow.com/a/74843981
#       ^
#       Hence for now.. executables and scripts are not run in parallel and..
#       .. .executables and .scripts is only for grouping tests (for..
#       .. convenience perhaps?)

define mm_add_releasezip_target=
$(MM_PROJECTNAME).$(MM_OS).zip: $(MM_RELEASEZIP)
	$(call mm_cli_zip,$$@,$(MM_RELEASEZIP))
endef

define mm_add_releaseinstallermsi_target=
$(MM_PROJECTNAME).msi: $(MM_RELEASEINSTALLER)
endef
define mm_add_releaseinstallerdeb_target=
$(MM_PROJECTNAME).deb: $(MM_RELEASEINSTALLER)
endef
define mm_add_releaseinstallersnap_target=
$(MM_PROJECTNAME).snap: $(MM_RELEASEINSTALLER)
endef

define mm_add_releaseinstaller_targets=
$(if $(OS),\
$(call mm_add_releaseinstaller_msi_target),\
$(call mm_add_releaseinstaller_deb_target)\
$(call mm_add_releaseinstaller_snap_target)\
)
endef

define mm_add_release_targets=
$(if $(MM_RELEASEZIP),$(call mm_add_releasezip_target),)
$(if $(MM_RELEASEINSTALLER),$(call mm_add_releaseinstaller_targets),)

.PHONY: release
release: $(MM_RELEASE)
	@echo Reminder.. did you run "make test" before running "make release"?
endef

define mm_add_clean_target=
.PHONY: clean
clean:
	$(foreach mm_add_clean_target_binarypart,$(MM_BINARYPARTS),$(MM_NEWLINE)	$(call mm_cli_rm,.makefile-mini/$(mm_add_clean_target_binarypart)))
	$(foreach mm_add_clean_target_binaryfilepath,$(MM_FILEPATH_PER_BINARY),$(MM_NEWLINE)	$(call mm_cli_rm,$(mm_add_clean_target_binaryfilepath)))
	$(foreach mm_add_clean_target_release,$(MM_RELEASE),$(MM_NEWLINE)	$(call mm_cli_rm,$(mm_add_clean_target_release)))
endef
# NOTE: ^
#       $(MM_NEWLINE)<tab>$(call <...>)
#                    ^
#                    to assure ends up in clean target?

# NOTE: $(1) == <mm_stop_parameters_t>
define mm_stop=
$(if $(filter undefined,$(origin MM_SAFETY)),,\
	$(call mm_check_add_makefile_parameters_t,$(0),$(1))\
)

$(eval $(call mm_add_default_target))

$(foreach mm_add_makefile_infoAboutSpvasmFromGlsl,$(MM_INFO_PER_SPVASM_FROM_GLSL),$\
$(eval $(call mm_add_spvasm_from_glsl_target,$(mm_add_makefile_infoAboutSpvasmFromGlsl)))$\
)

$(foreach mm_add_makefile_infoAboutShader,$(MM_INFO_PER_SHADER),$\
$(eval $(call mm_add_shader_target,$(mm_add_makefile_infoAboutShader)))$\
)

$(foreach mm_add_makefile_infoAboutOFromC,$(MM_INFO_PER_O_FROM_C),$\
$(eval $(call mm_add_o_from_c_target,$(mm_add_makefile_infoAboutOFromC)))$\
)

$(foreach mm_add_makefile_infoAboutLibrary,$(MM_INFO_PER_LIBRARY),$\
$(eval $(call mm_add_library_targets,$(mm_add_makefile_infoAboutLibrary)))$\
)

$(foreach mm_add_makefile_infoAboutExecutable,$(MM_INFO_PER_EXECUTABLE),$\
$(eval $(call mm_add_executable_targets,$(mm_add_makefile_infoAboutExecutable)))$\
)

$(eval $(call mm_add_test_target))

$(if $($(1).releasetypes),\
	$(eval MM_RELEASEBINARIES:=$(if $($(1).ifRelease.ignoredbinaries),$(call mm_filter_out_using_patterns,$($(1).ifRelease.ignoredbinaries),$(MM_NOTIGNOREDBINARIES)),$(MM_NOTIGNOREDBINARIES)))\
	$(eval MM_RELEASEFILES_PATTERNS:=$$($(1).ifRelease.additionalfiles))\
	$(if $(filter EMMReleasetype_Zip,$($(1).releasetypes)),\
		$(eval MM_RELEASEFILES_PATTERNS+=$(if $(MM_RELEASEFILES_PATTERNS), ,)$$($(1).ifRelease.ifZip.additionalfiles))\
	)\
	$(if $(filter EMMReleasetype_Installer,$($(1).releasetypes)),\
		$(eval MM_RELEASEFILES_PATTERNS+=$(if $(MM_RELEASEFILES_PATTERNS), ,)$$($(1).ifRelease.ifInstaller.additionalfiles))\
	)\
	$(eval MM_RELEASEFILES:=$(if $(MM_RELEASEFILES_PATTERNS),$(call mm_filter_using_patterns,$(MM_RELEASEFILES_PATTERNS),$(filter-out .makefile-mini/%,$(call mm_get_path_to_file_per_file))),))\
	$(if $(filter EMMReleasetype_Zip,$($(1).releasetypes)),\
		$(eval MM_RELEASEZIPBINARIES:=$(if $($(1).ifRelease.ifZip.ignoredbinaries),$(call mm_filter_out_using_patterns,$($(1).ifRelease.ifZip.ignoredbinaries),$(MM_RELEASEBINARIES)),$(MM_RELEASEBINARIES)))\
		$(eval MM_RELEASEZIPFILES:=$(if $($(1).ifRelease.ifZip.additionalfiles),$(call mm_filter_using_patterns,$($(1).ifRelease.ifZip.additionalfiles),$(MM_RELEASEFILES)),))\
		$(eval MM_RELEASEZIP:=$(MM_RELEASEZIPBINARIES) $(MM_RELEASEZIPFILES))\
		$(if $(MM_RELEASEZIP),\
			$(eval MM_RELEASE+=$(MM_PROJECTNAME).$(MM_OS).zip),\
			$(if $(filter undefined,$(origin MM_SAFETY)),,\
				$(info warning: release $(MM_PROJECTNAME).$(MM_OS).zip is cancelled as no files specified)\
			)\
		)\
	,)\
	$(if $(filter EMMReleasetype_Installer,$($(1).releasetypes)),\
		$(eval MM_RELEASEINSTALLERBINARIES:=$(if $($(1).ignoredbinaries),$(call mm_filter_out_using_patterns,$($(1).ifRelease.ifInstaller.ignoredbinaries),$(MM_RELEASEBINARIES)),$(MM_RELEASEBINARIES)))\
		$(eval MM_RELEASEINSTALLERFILES:=$(if $($(1).additionalfiles),$(call mm_filter_using_patterns,$($(1).ifRelease.ifInstaller.additionalfiles),$(MM_RELEASEFILES)),))\
		$(eval MM_RELEASEINSTALLER:=$(MM_RELEASEINSTALLERBINARIES) $(MM_RELEASEINSTALLERFILES))\
		$(if $(MM_RELEASEINSTALLER),\
			$(eval MM_RELEASE+=$(addprefix $(MM_PROJECTNAME),$(MM_RELEASEINSTALLER_EXTENSIONS))),\
			$(if $(filter undefined,$(origin MM_SAFETY)),,\
				$(info warning: release file(s) $(addprefix $(MM_PROJECTNAME),$(MM_RELEASEINSTALLER_EXTENSIONS)) is/are cancelled as no files specified)\
			)\
		)\
	,)\
,)
$(if $(MM_RELEASE),\
$(eval $(call mm_add_release_targets))\
,)

$(eval $(call mm_add_clean_target))
endef
# NOTE: ^
#       strip in MM_RELEASEFILES_PATTERNS:=$(strip <..>) because $(if..
#       .. $(MM_RELEASEFILES_PATTERNS),<..>)
# NOTE: ^
#       $$ in $(eval MM_RELEASEFILES_PATTERNS:=$$<..>) and $(eval..
#       .. MM_RELEASEFILES_PATTERNS:=<..>$$<..>) because patterns..
#       .. may contain $ which should not be directly supplied to eval
# NOTE: ^
#       $(if $(MM_RELEASE*),,$(eval MM_RELEASE:=$(filter-out <..>,$(MM_RELEASE))))
#       ^
#       if releasetype specified but no files.. don't make reelase
