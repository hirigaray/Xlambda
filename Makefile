CC ?= cc
CCLD ?= $(CC)

CFLAGS += -fPIC -g -Os -Wall -Wno-unused-function $(shell pkg-config --cflags guile-2.2 xcb)
LDFLAGS += $(shell pkg-config --libs guile-2.2 xcb)

OBJ := Xlambda.o

all: $(OBJ)
	$(CCLD) $(LDFLAGS) -o Xlambda $(OBJ)

clean:
	rm -f Xlambda $(OBJ)
