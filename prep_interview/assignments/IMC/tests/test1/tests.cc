#include "AreaVisitor.h"
#include "Circle.h"
#include "MathConstants.h"
#include "PerimeterVisitor.h"
#include "Rectangle.h"
#include "Triangle.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

// To compare double, we need a tolerance
static constexpr double TOL = 1e-9;

static bool near(double a, double b) { return std::abs(a - b) < TOL; }

int main() {
  AreaVisitor av;
  PerimeterVisitor pv;

  // --- Circle ---
  Circle c(5.0);
  c.accept(av);
  assert(near(av.area(), PI * 25.0));
  c.accept(pv);
  assert(near(pv.perimeter(), 2.0 * PI * 5.0));

  // --- Rectangle ---
  Rectangle r(4.0, 7.0);
  r.accept(av);
  assert(near(av.area(), 28.0));
  r.accept(pv);
  assert(near(pv.perimeter(), 22.0));

  // --- Triangle (3-4-5 right triangle, area = 6, perimeter = 12) ---
  Triangle t(3.0, 4.0, 5.0);
  t.accept(av);
  assert(near(av.area(), 6.0));
  t.accept(pv);
  assert(near(pv.perimeter(), 12.0));

  // --- Equilateral triangle, side=2, area = sqrt(3), perimeter = 6 ---
  Triangle eq(2.0, 2.0, 2.0);
  eq.accept(av);
  assert(near(av.area(), std::sqrt(3.0)));
  eq.accept(pv);
  assert(near(pv.perimeter(), 6.0));

  // --- Polymorphic dispatch through Shape* ---
  std::vector<std::unique_ptr<Shape>> shapes;
  shapes.push_back(std::make_unique<Circle>(1.0));
  shapes.push_back(std::make_unique<Rectangle>(2.0, 3.0));
  shapes.push_back(std::make_unique<Triangle>(3.0, 4.0, 5.0));

  double expectedArea[] = {PI, 6.0, 6.0};
  double expectedPerim[] = {2.0 * PI, 10.0, 12.0};
  for (std::size_t i = 0; i < shapes.size(); ++i) {
    shapes[i]->accept(av);
    assert(near(av.area(), expectedArea[i]));
    shapes[i]->accept(pv);
    assert(near(pv.perimeter(), expectedPerim[i]));
  }

  // --- Invalid construction ---
  try {
    Circle(-1.0);
    assert(false);
  } catch (const std::invalid_argument &) {
  }
  try {
    Rectangle(0.0, 5.0);
    assert(false);
  } catch (const std::invalid_argument &) {
  }
  try {
    Triangle(1.0, 2.0, 10.0);
    assert(false);
  } catch (const std::invalid_argument &) {
  }

  std::cout << "All tests passed.\n";
  return 0;
}