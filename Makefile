CC ?= cc
CCLD ?= $(CC)

CFLAGS += -fPIC -g -Os -Wall -Wno-unused-function `pkg-config --cflags guile-2.2 xcb`
LDFLAGS += `pkg-config --libs guile-2.2 xcb`

OBJ := Xlambda.o

all: $(OBJ)
	$(CCLD) $(LDFLAGS) -o Xlambda $(OBJ)

clean:
	rm -f Xlambda $(OBJ)
