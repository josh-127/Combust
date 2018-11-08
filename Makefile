TARGET	:= mycc

HFILES	:= \
	common.h \
	lexer.h

CFILES	:= \
	error.c \
	lexer.c \
	mycc.c \
	source.c

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): $(CFILES) $(HFILES)
	gcc -o$@ -ansi $^
