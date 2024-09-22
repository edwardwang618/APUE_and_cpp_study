#include <stdio.h>

#define PI 3.14159

#define SQUARE(x) ((x) * (x))

int main() {
  double radius = 5.0;
  double area = PI * SQUARE(radius);
  printf("The area of a circle with radius %f is %f\n", radius, area);
  return 0;
}