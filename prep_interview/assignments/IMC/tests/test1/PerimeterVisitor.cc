#include "PerimeterVisitor.h"
#include "Circle.h"
#include "MathConstants.h"
#include "Rectangle.h"
#include "Triangle.h"

void PerimeterVisitor::visit(const Circle &circle) {
  perimeter_ = 2.0 * PI * circle.radius();
}

void PerimeterVisitor::visit(const Rectangle &rectangle) {
  perimeter_ = 2.0 * (rectangle.width() + rectangle.height());
}

void PerimeterVisitor::visit(const Triangle &triangle) {
  perimeter_ = triangle.sideA() + triangle.sideB() + triangle.sideC();
}