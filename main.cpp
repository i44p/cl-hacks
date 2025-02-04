#include "adder.h"
#include <iomanip>
#include <vector>
#include <random>
#include "iostream"

std::random_device rd;
std::default_random_engine re1(rd());
std::uniform_int_distribution<int> rand_size(1, 16);
std::uniform_real_distribution<float> rand_data(0, 1);

std::vector<float> random_create() {
    size_t out_size = rand_size(re1);
    return std::vector<float>(out_size);
}

void random_fill(std::vector<float>& vec) {
    for (auto &elem : vec) {
        elem = rand_data(re1);
    }
}

void print_vec(std::vector<float> vec) {
    std::cout << std::setprecision(2);
    for (float& number: vec)
    {
        std::cout << number << "\t";
    }

    std::cout << std::endl;
}

void run_test_1(VectorfAdder& adder) {
    std::vector<float> vec1{1,2,3};
    print_vec(vec1);
    std::vector<float> vec2{4,5,6};
    print_vec(vec2);

    std::vector<float> result = *adder.Add(vec1, vec2);
    print_vec(result);
}

void run_test_2(VectorfAdder& adder) {
    std::vector<float> vec1 = random_create();
    random_fill(vec1);
    print_vec(vec1);
    std::vector<float> vec2(vec1.size());
    random_fill(vec2);
    print_vec(vec2);

    std::vector<float> result = *adder.Add(vec1, vec2);
    print_vec(result);
}


int run_tests(int argc, char **argv) {
    VectorfAdder adder("./add_kernel.cl");
    run_test_1(adder);
    run_test_2(adder);
    return 0;
}

int main(int argc, char **argv) {
  return run_tests(argc, argv);
}