# This makefile runs tests. It also runs `generate.sh` automatically, if the generated header is out of date.

override comma := ,

# A space-separated list of compilers.
# Normally those are detected automatically based on `COMPILER_PATTERNS`, but you can override them.
COMPILER := $(strip $(shell bash -c "compgen -c 'g++-'; compgen -c 'clang++'" | grep -Po '^((clan)?g\+\+(-[0-9]+)?|cl|clang-cl)(?=.exe)?$$' | sort -hr -t- -k2 | uniq))
# Adjust the compiler list to test the `-fms-{compatibility,extensions}` flags too.
COMPILER := $(foreach x,$(COMPILER),$(if $(filter g++%,$x),$x $x$(comma)-fms-extensions,$(if $(filter clang++%,$x),$x $x$(comma)-fms-compatibility,$x)))

# The source file used for code generation.
INPUT=macro_sequence_for.codegen_input.h
# The result of the generation.
OUTPUT=include/macro_sequence_for.h

.PHONY: tests
tests: tests.cpp $(OUTPUT)
	@true $(foreach x,$(COMPILER),&& echo -n "$(subst $(comma), ,$x) " && $(subst $(comma), ,$x) -std=c++20 -Wall -Wextra -pedantic-errors -Iinclude $< -c -o $(shell mktemp) && echo "- OK")

$(OUTPUT): $(INPUT) generate.sh
	@mkdir -p "$(dir $(OUTPUT))"
	./generate.sh
