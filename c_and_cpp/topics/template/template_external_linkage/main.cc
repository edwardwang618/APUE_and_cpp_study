#include "pi.h"
#include <iostream>

void print_float_pi();
void print_double_pi();

int main() {
    print_float_pi();
    print_double_pi();
    std::cout << "pi<long double> from main: " << pi<long double> << std::endl;
    return 0;
}