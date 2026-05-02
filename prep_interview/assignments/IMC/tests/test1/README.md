# Test 1 — Visitor Pattern (Shapes)

## Overview

Classic Visitor pattern over a Shape hierarchy (Circle, Rectangle, Triangle).
Shapes hold geometry and accept visitors; visitors implement operations externally.

## Design Decisions

- **Triangle** takes three side lengths rather than base/height — more general,
  and Heron's formula handles area cleanly.
- Constructors validate inputs (positive dimensions, triangle inequality) and
  throw `std::invalid_argument` on failure.
- `MathConstants.h` provides a shared PI constant used by both visitors and tests.

## Extensibility

- **New operation:** Add a new `ShapeVisitor` subclass (like `PerimeterVisitor`).
  No changes to any Shape class needed.
- **New shape:** Add the class, then add a `visit()` overload to `ShapeVisitor`
  and each concrete visitor. This is the known trade-off of the pattern.

## Build & Run

    make            # build demo + test runner
    make test       # run tests
    ./visitor_demo  # run the demo
    make clean
