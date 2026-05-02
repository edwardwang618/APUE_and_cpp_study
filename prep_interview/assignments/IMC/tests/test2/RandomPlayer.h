#pragma once

#include "Player.h"

#include <cstdlib>
#include <ctime>
#include <string>

class RandomPlayer : public Player {
public:
  explicit RandomPlayer(std::string name) : name_(std::move(name)) {}

  Move chooseMove() override {
    static constexpr Move moves[] = {Move::Rock, Move::Paper, Move::Scissors};
    return moves[std::rand() % 3];
  }

  std::string name() const override { return name_; }

private:
  std::string name_;
};