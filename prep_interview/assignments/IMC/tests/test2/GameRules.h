#pragma once

#include "GameResult.h"
#include "Move.h"

#include <string>
#include <vector>

class GameRules {
public:
  struct Rule {
    Move winner;
    Move loser;
    std::string verb;
  };

  void addRule(Move winner, Move loser, std::string verb);

  // Result is from p1's perspective
  GameResult judge(Move p1, Move p2) const;
  std::string describe(Move winner, Move loser) const;

  static GameRules standard();

private:
  std::vector<Rule> rules_;
};