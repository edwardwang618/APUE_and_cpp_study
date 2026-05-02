#pragma once

#include "Move.h"

#include <string>

class Player {
public:
  virtual ~Player() = default;
  virtual Move chooseMove() = 0;
  virtual std::string name() const = 0;
};