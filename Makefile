CC = gcc
LOADER-SRC        = $(addprefix src/,OpenLibrary.c MapLibrary.c RelocLibrary.c \
						FindSymbol.c RuntimeResolve.c trampoline.S InitLibrary.c)
LOADER-SAMPLE-SRC = $(addprefix src/orig/,OpenLibrary.c MapLibrary.c RelocLibrary.c \
						FindSymbol.c RuntimeResolve.c trampoline.S InitLibrary.c)
TST-LIBS          = $(addprefix test_lib/,lib1.so SimpleMul.so SimpleIni.so SimpleData.so)

CFLAGS = -g -shared -fPIC
LDFLAGS = -ldl
CUSTOM-LDR = -L./build -Wl,-rpath,./build -l:loader-sample.so
REAL-LDR = -L./build -Wl,-rpath,./build -l:loader.so

all: loader libs test

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

# test program used as slave of autograder
build/testrun: testrun.c
	$(CC) -g -o $@ $< $(REAL-LDR)
# $(CC) -g -o $@ $< $(CUSTOM-LDR)

test: build/run-dlopen build/run-openlib

build/run-dlopen: test.c
	$(CC) -g -DUSE_DLOPEN=1 -o $@ $< $(LDFLAGS)

build/run-openlib: test.c
	$(CC) -g -o $@ $< $(REAL-LDR)
#  $(CC) -g -o $@ $< $(CUSTOM-LDR)

clean:
	rm -f build/loader.so build/loader-sample.so build/run-dlopen build/run-openlib
