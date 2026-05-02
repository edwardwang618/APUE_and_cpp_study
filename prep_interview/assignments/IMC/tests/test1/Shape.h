#pragma once

class ShapeVisitor;

// An abstract class for all shapes. Each concrete shape must implement accept()
// to call the appropriate visit() overload.
class Shape {
public:
  virtual ~Shape() = default;
  virtual void accept(ShapeVisitor &visitor) const = 0;
};