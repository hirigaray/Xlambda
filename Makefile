CC      = cc
LD      = $(CC)

CFLAGS  = -fPIC -Os
CFLAGS += -Wall -Wno-unused-function
CFLAGS += -I/usr/local/include/guile/2.2 -I/usr/local/include -pthread
LDFLAGS = -L/usr/local/lib -pthread -lguile-2.2 -lgc
CFLAGS += -I/usr/X11R6/include
LDFLAGS += -L/usr/X11R6/lib -lxcb


all:
	$(CC) $(CFLAGS) $(LDFLAGS) Xlambda.c -o Xlambda
