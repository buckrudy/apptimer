CC = gcc
CFLAGS += -I. -lpthread
LDFLAGS += -L.
INSTALL = install
PREFIX := /usr

all: test


test: test.c libapptimer.so
	$(CC) $< -o $@ -lapptimer $(CFLAGS) $(LDFLAGS)


libapptimer.so: apptimer.c
	$(CC) -fPIC -g -c -Wall $<
	$(CC) -shared -Wl,-soname,libapptimer.so -o $@ apptimer.o


install:
	$(INSTALL) libapptimer.so $(PREFIX)/lib/
	$(INSTALL) apptimer.h $(PREFIX)/include/

clean:
	$(RM) apptimer.o test libapptimer.so

.PHONY: all test install

