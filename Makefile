CC = gcc
LOADER-SRC = src/OpenLibrary.c

CFLAGS = -g
LDFLAGS = -ldl

build/loader: $(LOADER-SRC) | build
	$(CC) $(CFLAGS) $(LOADER-SRC) -o $@ $(LDFLAGS)

# @ can suppress the echo of the command
build:
	@mkdir -p $@
