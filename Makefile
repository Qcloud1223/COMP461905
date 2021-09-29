CC = gcc
LOADER-SRC        = src/OpenLibrary.c
LOADER-SAMPLE-SRC = $(addprefix src/orig/,OpenLibrary.c MapLibrary.c RelocLibrary.c \
						FindSymbol.c RuntimeResolve.c trampoline.S)
TST-LIBS          = $(addprefix test_lib/,lib1.so SimpleMul.so SimpleIni.so)

CFLAGS = -g -shared -fPIC
LDFLAGS = -ldl
CUSTOM-LDR = -L./build -Wl,-rpath,./build -l:loader-sample.so

loader: build/loader.so
loader-sample: build/loader-sample.so
.PHONY: test

build/loader.so: $(LOADER-SRC) | build
	$(CC) $(CFLAGS) $(LOADER-SRC) -o $@ $(LDFLAGS)

# sample loader used for skeleton verification
build/loader-sample.so: $(LOADER-SAMPLE-SRC) | build
	$(CC) $(CFLAGS) $(LOADER-SAMPLE-SRC) -o $@ $(LDFLAGS)

libs: $(TST-LIBS)

$(TST-LIBS): %.so: %.c
	$(CC) $(CFLAGS) $< -o $@

# @ can suppress the echo of the command
build:
	@mkdir -p $@

build/testrun: testrun.c
	$(CC) -g -o $@ $< $(CUSTOM-LDR)

test: build/run-dlopen build/run-openlib

build/run-dlopen: test.c
	$(CC) -g -DUSE_DLOPEN=1 -o $@ $< $(LDFLAGS)

build/run-openlib: test.c
	$(CC) -g -o $@ $< $(CUSTOM-LDR)

clean:
	rm -f build/loader.so build/loader-sample.so build/run-dlopen build/run-openlib
