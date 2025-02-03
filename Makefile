CXX=clang++
CXXFLAGS=-std=c++20

default:
	$(CXX) $(CXXFLAGS) main.cpp -lOpenCL -o main

run: default
	./main

clean:
	rm main
