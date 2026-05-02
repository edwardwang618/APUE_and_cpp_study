#pragma once

class Circle;
class Rectangle;
class Triangle;

// Implement this interface to add a new operation over shapes.
class ShapeVisitor {
public:
  virtual ~ShapeVisitor() = default;

  virtual void visit(const Circle &circle) = 0;
  virtual void visit(const Rectangle &rectangle) = 0;
  virtual void visit(const Triangle &triangle) = 0;
};