CC      = gcc
LD      = $(CC)
OS      = $(shell uname)

CFLAGS  = -fPIC -Os -Wall
CFLAGS += `pkg-config --cflags xcb guile-3.0`

LDFLAGS = `pkg-config --libs xcb guile-3.0`


all:
	$(CC) $(CFLAGS) $(LDFLAGS) Xlambda.c -o Xlambda
