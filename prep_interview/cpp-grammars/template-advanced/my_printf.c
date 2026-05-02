#include <stdarg.h>
#include <stdio.h>

int print_int(int val) {
  int written = 0;
  if (val < 0) {
    putchar('-');
    written++;
    // handle INT_MIN: can't negate it safely
    if (val == (-2147483647 - 1)) {
      written += print_int(val / 10 * -1);
      putchar('0' + -(val % 10));
      return written + 1;
    }
    val = -val;
  }
  if (val >= 10) {
    written += print_int(val / 10);
  }
  putchar('0' + val % 10);
  return written + 1;
}

int print_long(long val) {
  int written = 0;
  if (val < 0) {
    putchar('-');
    written++;
    val = -val;
  }
  if (val >= 10) {
    written += print_long(val / 10);
  }
  putchar('0' + (int)(val % 10));
  return written + 1;
}

int print_str(const char *s) {
  int written = 0;
  while (*s) {
    putchar(*s++);
    written++;
  }
  return written;
}

int print_hex(unsigned val) {
  const char *hex = "0123456789abcdef";
  int written = 0;
  if (val >= 16) {
    written += print_hex(val / 16);
  }
  putchar(hex[val % 16]);
  return written + 1;
}

int print_double(double val) {
  int written = 0;
  if (val < 0) {
    putchar('-');
    written++;
    val = -val;
  }
  int integer_part = (int)val;
  double frac = val - integer_part;

  written += print_int(integer_part);
  putchar('.');
  written++;

  // 6 decimal places (default printf precision)
  for (int i = 0; i < 6; i++) {
    frac *= 10;
    int digit = (int)frac;
    putchar('0' + digit);
    frac -= digit;
    written++;
  }
  return written;
}

int my_printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int written = 0;

  while (*fmt) {
    if (*fmt != '%') {
      putchar(*fmt++);
      written++;
      continue;
    }
    fmt++;

    switch (*fmt++) {
    case 'd': {
      int val = va_arg(args, int);
      written += print_int(val);
      break;
    }
    case 'l': {
      if (*fmt == 'd') {
        fmt++;
        long val = va_arg(args, long);
        written += print_long(val);
      }
      break;
    }
    case 's': {
      char *val = va_arg(args, char *);
      written += print_str(val);
      break;
    }
    case 'f': {
      double val = va_arg(args, double);
      written += print_double(val);
      break;
    }
    case 'x': {
      unsigned val = va_arg(args, unsigned);
      written += print_hex(val);
      break;
    }
    case '%': {
      putchar('%');
      written++;
      break;
    }
    }
  }

  va_end(args);
  return written;
}

int main() {
  my_printf("%d\n", 42);
  my_printf("%d\n", -123);
  my_printf("%s %s\n", "hello", "world");
  my_printf("%x\n", 255);
  my_printf("%f\n", 3.14159);
  my_printf("100%% done\n");
  return 0;
}