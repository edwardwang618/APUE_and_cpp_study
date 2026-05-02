#pragma once

#include "Shape.h"
#include "ShapeVisitor.h"

#include <stdexcept>

class Triangle final : public Shape {
public:
  Triangle(double a, double b, double c) : a_(a), b_(b), c_(c) {
    if (a_ <= 0.0 || b_ <= 0.0 || c_ <= 0.0) {
      throw std::invalid_argument("Triangle side lengths must be positive");
    }
    if (!isValidTriangle(a_, b_, c_)) {
      throw std::invalid_argument(
          "Side lengths violate the triangle inequality");
    }
  }

  void accept(ShapeVisitor &visitor) const override { visitor.visit(*this); }

  double sideA() const { return a_; }
  double sideB() const { return b_; }
  double sideC() const { return c_; }

private:
  static bool isValidTriangle(double a, double b, double c) {
    return (a + b > c) && (a + c > b) && (b + c > a);
  }

  double a_;
  double b_;
  double c_;
};