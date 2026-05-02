#pragma once

#include <stdexcept>
#include <string>

enum class Move { Rock, Paper, Scissors };

inline std::string toString(Move m) {
  switch (m) {
  case Move::Rock:
    return "Rock";
  case Move::Paper:
    return "Paper";
  case Move::Scissors:
    return "Scissors";
  }
  throw std::invalid_argument("Unknown move");
}