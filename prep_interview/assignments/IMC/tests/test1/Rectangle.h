#pragma once

#include "Shape.h"
#include "ShapeVisitor.h"

#include <stdexcept>

class Rectangle final : public Shape {
public:
  Rectangle(double width, double height) : width_(width), height_(height) {
    if (width_ <= 0.0 || height_ <= 0.0) {
      throw std::invalid_argument("Rectangle dimensions must be positive");
    }
  }

  void accept(ShapeVisitor &visitor) const override { visitor.visit(*this); }

  double width() const { return width_; }
  double height() const { return height_; }

private:
  double width_;
  double height_;
};