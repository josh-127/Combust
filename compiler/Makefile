TARGET		:= compile

CXXFLAGS	:= -g -Wall -Wextra

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

$(TARGET): $(CCFILES) $(HHFILES)
	g++ $(CXXFLAGS) -o$@ $(CCFILES)
