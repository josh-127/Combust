TARGET	:= ptc-cl

CFLAGS	:= -g -ansi

HFILES	:= \
	common.h \
	lexer.h

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
	gcc $(CFLAGS) -o$@ $^
