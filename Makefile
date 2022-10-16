# DO NOT MODIFY

CC = gcc
LOADER-SRC        = $(addprefix src/,OpenLibrary.c MapLibrary.c RelocLibrary.c \
						FindSymbol.c RuntimeResolve.c trampoline.S InitLibrary.c)
LOADER-SAMPLE-SRC = $(addprefix src/orig/,OpenLibrary.c MapLibrary.c RelocLibrary.c \
						FindSymbol.c RuntimeResolve.c trampoline.S InitLibrary.c)
TST-LIBS          = $(addprefix test_lib/,lib1.so SimpleMul.so SimpleIni.so SimpleData.so)

USE-CUSTOM-LDR ?= F

LOADER-SRC += util/shim.c

LOADER-SAMPLE-SRC += util/shim.c

# https://wiki.ubuntu.com/ToolChain/CompilerFlags
# Thanks to Ubuntu Wiki, I know that control flow protection was introduced since 19.04
# But I can't tell the accurate gcc version for it is even OS-dependent
# At the end of day I decide to keep things broken and enable it only on 20.04  
CFLAGS = -g -shared -fPIC
GCCVERSIONGTEQ9 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 9)
ifeq "$(GCCVERSIONGTEQ9)" "1"
	CFLAGS += -fcf-protection=none
endif
# let the functions use the version as specified in source
CFLAGS += -fno-builtin

LDFLAGS = -ldl
LDR = -L./build -Wl,-rpath,./build
ifeq "$(USE-CUSTOM-LDR)" "F"
	LDR += -l:loader.so
	ALL-OBJ = loader libs test
else
	LDR += -l:loader-sample.so
	ALL-OBJ = loader-sample libs test
endif

all: $(ALL-OBJ)

# sample loader and skeleton loader library
loader: build/loader.so
loader-sample: build/loader-sample.so
.PHONY: test

build/loader.so: $(LOADER-SRC) | build
	$(CC) $(CFLAGS) $(LOADER-SRC) -o $@ $(LDFLAGS)

build/loader-sample.so: $(LOADER-SAMPLE-SRC) | build
	$(CC) $(CFLAGS) $(LOADER-SAMPLE-SRC) -o $@ $(LDFLAGS)

# test libs for autograder
libs: $(TST-LIBS) test_lib/SimpleDep.so test_lib/IndirectDep.so

$(TST-LIBS): %.so: %.c
	$(CC) $(CFLAGS) $< -o $@

test_lib/SimpleDep.so: test_lib/SimpleDep.c
	$(CC) $(CFLAGS) $< -o $@ -L./test_lib -Wl,-rpath,./test_lib -l:SimpleMul.so

test_lib/IndirectDep.so: test_lib/IndirectDep.c
	$(CC) $(CFLAGS) $< -o $@ -L./test_lib -Wl,-rpath,./test_lib -l:SimpleDep.so

# @ can suppress the echo of the command
build:
	@mkdir -p $@

# build/testrun: testrun.c
# 	$(CC) -g -o $@ $< $(REAL-LDR)
# $(CC) -g -o $@ $< $(CUSTOM-LDR)

# test program used as slave of autograder
test: build/run-dlopen build/run-openlib

build/run-dlopen: test.c
	$(CC) -g -DUSE_DLOPEN=1 -o $@ $< $(LDFLAGS)

build/run-openlib: test.c
	$(CC) -g -o $@ $< $(LDR)

# shim layer that is used to intercept calls to critical functions
# shim: build/libshim.so 

# build/libshim.so: util/shim.c 
# 	$(CC) $(CFLAGS) -o $@ $< $(LDR) $(LDFLAGS)

clean:
	rm -f build/loader.so build/loader-sample.so build/run-dlopen build/run-openlib

cleanlib:
	rm -f test_lib/*.so