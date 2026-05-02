#include "AreaVisitor.h"
#include "Circle.h"
#include "MathConstants.h"
#include "Rectangle.h"
#include "Triangle.h"

#include <cmath>

void AreaVisitor::visit(const Circle &circle) {
  double r = circle.radius();
  area_ = PI * r * r;
}

void AreaVisitor::visit(const Rectangle &rectangle) {
  area_ = rectangle.width() * rectangle.height();
}

void AreaVisitor::visit(const Triangle &triangle) {
  double a = triangle.sideA();
  double b = triangle.sideB();
  double c = triangle.sideC();
  double s = (a + b + c) / 2.0;
  // Heron's formula
  area_ = std::sqrt(s * (s - a) * (s - b) * (s - c));
}