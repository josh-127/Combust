TARGET		:= compile

CXXFLAGS	:= -g -std=gnu++1z -Wall -Wextra

HHFILES	:= \
	lexer.hh \
	logger.hh \
	source.hh \
	syntax.hh \
	syntax-kinds.def

CCFILES	:= \
	main.cc \
	lexer.cc \
	logger.cc \
	source.cc \
	syntax.cc

.PHONY: all clean

all: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): $(CCFILES) $(HHFILES) .makefile-config
	`cat .makefile-config` $(CXXFLAGS) -o$@ $(CCFILES)

.makefile-config:
	./configure
