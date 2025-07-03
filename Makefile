#
# To build ROM only
# 1) make rom
#
CC       = gcc
CFLAGS   = -std=c17 -O2 -Wall -Wno-unused-value -Wno-unused-variable 
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall

EXE  = tests/eforth1
ASM  = src/eforth_asm.o
OBJS = \
	src/eforth_core.o \
	src/eforth_vm.o   \
	src/eforth_rom.o  \
	src/eforth1.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

all: rom $(EXE)

rom: $(ASM)
	$(CC) -o tests/ef_$@ $(ASM)
	echo Geneating eForth ROM...
	tests/ef_$@ > src/eforth_rom.c

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm $(EXE) $(OBJS) $(ASM)


