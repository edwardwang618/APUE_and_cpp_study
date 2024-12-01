# 0 "main.c"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "main.c"
# 1 "add.h" 1
       

# 1 "sub.h" 1


int sub(int x, int y) { return x - y; }
# 4 "add.h" 2

int add(int a, int b) { return a + b; }
# 2 "main.c" 2

int main() { int x = add(1, 2), y = sub(3, 4); }
