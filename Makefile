CC = gcc
LOADER-SRC        = src/OpenLibrary.c
LOADER-SAMPLE-SRC = $(addprefix src/orig/,OpenLibrary.c MapLibrary.c RelocLibrary.c \
						FindSymbol.c RuntimeResolve.c trampoline.S)
TST-LIBS          = test_lib/SimpleMul.so

CFLAGS = -g -shared -fPIC
LDFLAGS = -ldl

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
	$(CC) -g -o $@ $< -L./build -Wl,-rpath,./build -l:loader-sample.so
