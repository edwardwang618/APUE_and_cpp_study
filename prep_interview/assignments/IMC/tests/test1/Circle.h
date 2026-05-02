#pragma once

#include "Shape.h"
#include "ShapeVisitor.h"

#include <stdexcept>

class Circle final : public Shape {
public:
  explicit Circle(double radius) : radius_(radius) {
    if (radius_ <= 0.0) {
      throw std::invalid_argument("Circle radius must be positive");
    }
  }

  void accept(ShapeVisitor &visitor) const override { visitor.visit(*this); }

  double radius() const { return radius_; }

private:
  double radius_;
};