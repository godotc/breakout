.PHONY: cfg

cfg:
	xmake f -m debug  
	xmake project -k compile_commands

# CC:=

exe_suffix:= # linux .out and windows .exe
ifeq ($(OS), Windows_NT)
	exe_suffix:= .exe
else
	exe_suffix:= .out
endif


# About manual test
manual_test_path_prefix=./test/manual
test_target:= $(manual_test_path_prefix)/$(T)_test
test_output= $(test_target)$(exe_suffix)

manual_test: 
	$(CC) $(test_target).cc -o $(test_output)
	$(test_output)

manual_test_with_deps: 
	xmake build manual_test_$(T) && xmake run manual_test_$(T)
