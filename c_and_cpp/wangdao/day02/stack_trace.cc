#include <cxxabi.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void print_stacktrace() {
  const int max_frames = 64;
  void* addrlist[max_frames + 1];

  // Retrieve current stack addresses
  int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

  if (addrlen == 0) {
    printf("No stack trace available.\n");
    return;
  }

  // Resolve addresses into strings containing "function + offset"
  char** symbol_list = backtrace_symbols(addrlist, addrlen);

  // Iterate over the returned symbol lines
  for (int i = 1; i < addrlen; i++) {
    char *begin_name = NULL, *begin_offset = NULL, *end_offset = NULL;

    // Find parentheses and +address offset surrounding the mangled name
    for (char* p = symbol_list[i]; *p; ++p) {
      if (*p == '(')
        begin_name = p;
      else if (*p == '+')
        begin_offset = p;
      else if (*p == ')' && begin_offset) {
        end_offset = p;
        break;
      }
    }

    if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
      *begin_name++ = '\0';
      *begin_offset++ = '\0';
      *end_offset = '\0';

      // Demangle the function name
      int status;
      char* demangled_name =
          abi::__cxa_demangle(begin_name, NULL, NULL, &status);
      char* func_name = (status == 0) ? demangled_name : begin_name;

      printf("  [%d] %s : %s+%s\n", i, symbol_list[i], func_name, begin_offset);

      free(demangled_name);
    } else {
      // Couldn't parse the line, print the whole line
      printf("  [%d] %s\n", i, symbol_list[i]);
    }
  }
  free(symbol_list);
}

void functionC() { print_stacktrace(); }

void functionB() { functionC(); }

void functionA() { functionB(); }

int main() {
  functionA();
  return 0;
}
