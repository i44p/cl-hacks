#include "adder.h"
#include <vector>
#include <random>
#include "iostream"

std::random_device rd;

void print_vec(std::vector<float> vec) {
    for (float number: vec)
    {
        std::cout << number << " ";
    }

    std::cout << std::endl;
}

void run_test_1() {
    std::vector<float> vec1{1,2,3};
    print_vec(vec1);
    std::vector<float> vec2{4,5,6};
    print_vec(vec2);
    
    VectorfAdder adder("./add_kernel.cl");

    std::vector<float> result = *adder.Add(vec1, vec2);
    print_vec(result);
}


int run_tests(int argc, char **argv) {
    run_test_1();
    return 0;
}

int main(int argc, char **argv) {
  return run_tests(argc, argv);
}