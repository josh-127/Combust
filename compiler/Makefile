
#
# Outputs
# =====================================================================
APP_TARGET	:= compile
TEST_TARGET	:= test-compiler


#
# Application Build Configuration
# =====================================================================
APP_CXXFLAGS	:= -g -std=gnu++1z -Wall -Wextra

APP_HHFILES	:= \
	backtracking-lexer.hh \
	code-lexer.hh \
	language-parser.hh \
	lexer.hh \
	logger.hh \
	preprocessor-lexer.hh \
	source.hh \
	syntax.hh \
	syntax-kinds.def

APP_CCFILES	:= \
	backtracking-lexer.cc \
	code-lexer.cc \
	language-parser.cc \
	logger.cc \
	preprocessor-lexer.cc \
	source.cc \
	syntax.cc

APP_ENTRY	:= main.cc


#
# Unit-Test Build Configuration
# =====================================================================
TEST_CXXFLAGS	:= \
	$(APP_CXXFLAGS) \
	-isystem vendor/Catch2 \
	-pthread \
	-D_VARIADIC_MAX=10

TEST_HHFILES	:= \
	$(APP_HHFILES)

TEST_CCFILES	:= \
	$(APP_CCFILES) \
	unit-tests/code-lexer-test.cc \
	unit-tests/expression-parser-test.cc \
	unit-tests/preprocessor-lexer-test.cc

TEST_ENTRY	:= unit-tests/main.cc


#
# Dependencies
# =====================================================================
.PHONY: all check clean

all: $(APP_TARGET)

check: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(APP_TARGET) $(TEST_TARGET)

$(APP_TARGET): $(APP_CCFILES) $(APP_HHFILES) .makefile-config
	`cat .makefile-config` $(APP_CXXFLAGS) -o$@ $(APP_CCFILES) $(APP_ENTRY)

$(TEST_TARGET): $(TEST_CCFILES) $(TEST_HHFILES) .makefile-config
	`cat .makefile-config` $(TEST_CXXFLAGS) -o$@ $(TEST_CCFILES) $(TEST_ENTRY)

.makefile-config:
	./configure
