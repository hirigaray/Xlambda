CC      = cc
LD      = $(CC)
OS      = $(shell uname)
GUILE_VERSION ?= 2.2

CFLAGS  = -fPIC -Os
CFLAGS += -Wall -Wno-unused-function
ifeq ($(OS),Linux)
	CFLAGS += -I/usr/include/guile/$(GUILE_VERSION) -pthread
else
	CFLAGS += -I/usr/local/include/guile/$(GUILE_VERSION) -I/usr/local/include -pthread
endif
LDFLAGS = -L/usr/local/lib -pthread -lguile-$(GUILE_VERSION) -lgc
CFLAGS += -I/usr/X11R6/include
LDFLAGS += -L/usr/X11R6/lib -lxcb


all:
	$(CC) $(CFLAGS) $(LDFLAGS) Xlambda.c -o Xlambda
