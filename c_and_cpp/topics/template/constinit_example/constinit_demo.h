#pragma once
#include <string>

// OK: global variable with constinit
constinit int global_var = 42;

// OK: static variable with constinit
static constinit const char* program_name = "MyProgram";

class ConstInitExample {
public:
    // OK: static class member with constinit
    static constinit int static_member;
    
    // This would be illegal if uncommented:
    /*
    constinit int local_var = 10;  // ERROR: constinit can only be used with static or thread local storage duration
    */
    
    // Thread-local storage with constinit
    static constinit thread_local int thread_counter;
};

// Definition of static member
constinit int ConstInitExample::static_member = 100;

// Definition of thread-local member
constinit thread_local int ConstInitExample::thread_counter = 0;

// Example of what's NOT allowed:
/*
void illegal_example() {
    constinit int local = 42;  // ERROR: local variables can't be constinit
    static constinit int size = rand();  // ERROR: not a constant initializer
    constinit int arr[global_var];  // ERROR: VLA size not allowed
}
*/
