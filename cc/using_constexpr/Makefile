SIM_ERROR ?= 0

all: main.bin

main.bin: main.cc
	g++ -DSIM_ERROR=$(SIM_ERROR) main.cc -o main.bin

clean:
	rm -f main.bin