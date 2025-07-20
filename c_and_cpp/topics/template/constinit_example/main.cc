#include "constinit_demo.h"
#include <iostream>
#include <thread>
#include <vector>

void thread_function() {
    // Each thread gets its own copy of thread_counter
    ConstInitExample::thread_counter++;
    std::cout << "Thread " << std::this_thread::get_id() 
              << " counter: " << ConstInitExample::thread_counter << std::endl;
}

int main() {
    // Access global constinit variable
    std::cout << "Global var: " << global_var << std::endl;
    
    // Access static constinit variable
    std::cout << "Program name: " << program_name << std::endl;
    
    // Access static class member with constinit
    std::cout << "Static member: " << ConstInitExample::static_member << std::endl;
    
    // Demonstrate thread-local constinit variable
    std::vector<std::thread> threads;
    for(int i = 0; i < 3; ++i) {
        threads.emplace_back(thread_function);
    }
    
    for(auto& t : threads) {
        t.join();
    }
    
    return 0;
}
