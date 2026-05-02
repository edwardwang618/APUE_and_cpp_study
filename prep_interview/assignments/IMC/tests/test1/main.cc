#include "AreaVisitor.h"
#include "Circle.h"
#include "Rectangle.h"
#include "Triangle.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

int main() {
  std::vector<std::unique_ptr<Shape>> shapes;
  shapes.push_back(std::make_unique<Circle>(5.0));
  shapes.push_back(std::make_unique<Circle>(1.0));
  shapes.push_back(std::make_unique<Rectangle>(4.0, 7.0));
  shapes.push_back(std::make_unique<Rectangle>(10.0, 2.5));
  shapes.push_back(std::make_unique<Triangle>(3.0, 4.0, 5.0));
  shapes.push_back(std::make_unique<Triangle>(5.0, 5.0, 5.0));

  AreaVisitor area;

  std::cout << std::fixed << std::setprecision(4);

  for (const auto &shape : shapes) {
    shape->accept(area);
    std::cout << "area = " << area.area() << "\n";
  }

  return 0;
}