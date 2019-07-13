CC      = cc
LD      = $(CC)
OS      = $(shell uname)
GUILE_VERSION ?= 2.2
GUILE_INCLUDE = $(OS)
GUILE_INCLUDE := $(GUILE_INCLUDE:Linux=/usr/include/guile/$(GUILE_VERSION))
GUILE_INCLUDE := $(GUILE_INCLUDE:$(OS)=/usr/local/include/guile/$(GUILE_VERSION))

CFLAGS  = -fPIC -Os
CFLAGS += -Wall
CFLAGS += -I$(GUILE_INCLUDE) -I/usr/local/include -pthread
LDFLAGS = -L/usr/local/lib -pthread -lguile-$(GUILE_VERSION) -lgc
CFLAGS += -I/usr/X11R6/include
LDFLAGS += -L/usr/X11R6/lib -lxcb


all:
	$(CC) $(CFLAGS) $(LDFLAGS) Xlambda.c -o Xlambda
