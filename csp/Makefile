SOURCE_DIR=src

all:
	g++ ${SOURCE_DIR}/*.cpp -o exe -std=c++14 -O3 -Wall -Wextra -Wpedantic -Werror -lglpk -L${SOURCE_DIR}

run: all
	./exe instances/e3

