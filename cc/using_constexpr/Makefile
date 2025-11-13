SIM_ERROR ?= 0
CXX_STANDARD ?= c++17

all: main.bin

main.bin: main.cc
	g++ -std=$(CXX_STANDARD) -DSIM_ERROR=$(SIM_ERROR) main.cc -o main.bin

clean:
	rm -f main.bin