TARGET	:= ptc-cl

CFLAGS	:= -g -ansi -Wall -Wextra -pedantic-errors

HFILES	:= \
	common.h \
	lexer.h \
	syntax.h \
	syntax-kinds.def

CFILES	:= \
	error.c \
	lexer.c \
	ptc-cl.c \
	source.c

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): $(CFILES) $(HFILES)
	gcc $(CFLAGS) -o$@ $(CFILES)
