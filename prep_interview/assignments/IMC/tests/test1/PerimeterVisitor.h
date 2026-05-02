#pragma once

#include "ShapeVisitor.h"

class PerimeterVisitor final : public ShapeVisitor {
public:
  void visit(const Circle &circle) override;
  void visit(const Rectangle &rectangle) override;
  void visit(const Triangle &triangle) override;

  double perimeter() const { return perimeter_; }

private:
  double perimeter_ = 0.0;
};