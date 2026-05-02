#pragma once

#include "GameRules.h"
#include "Player.h"

#include <memory>

class Game {
public:
  Game(GameRules rules, std::unique_ptr<Player> p1, std::unique_ptr<Player> p2);
  void playRounds(int n);

private:
  GameRules rules_;
  std::unique_ptr<Player> p1_;
  std::unique_ptr<Player> p2_;
  int wins1_ = 0;
  int wins2_ = 0;
  int draws_ = 0;

  void playRound();
  void printSummary() const;
};