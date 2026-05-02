#pragma once

#include "ShapeVisitor.h"

class AreaVisitor final : public ShapeVisitor {
public:
  void visit(const Circle &circle) override;
  void visit(const Rectangle &rectangle) override;
  void visit(const Triangle &triangle) override;

  double area() const { return area_; }

private:
  double area_ = 0.0;
};