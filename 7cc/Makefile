TARGET	:= 7cc

CFLAGS	:= -g -ansi -Wall -Wextra -pedantic-errors

HFILES	:= \
	lexer.h \
	logger.h \
	source.h \
	syntax.h \
	syntax-kinds.def

CFILES	:= \
	7cc.c \
	lexer.c \
	logger.c \
	source.c \
	syntax.c

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): $(CFILES) $(HFILES)
	gcc $(CFLAGS) -o$@ $(CFILES)
