#include "GameRules.h"

#include <stdexcept>

void GameRules::addRule(Move winner, Move loser, std::string verb) {
  rules_.push_back({winner, loser, std::move(verb)});
}

GameResult GameRules::judge(Move p1, Move p2) const {
  if (p1 == p2)
    return GameResult::Draw;

  for (const auto &rule : rules_) {
    if (rule.winner == p1 && rule.loser == p2)
      return GameResult::Win;
    if (rule.winner == p2 && rule.loser == p1)
      return GameResult::Loss;
  }
  throw std::invalid_argument("No rule covers this move combination");
}

std::string GameRules::describe(Move winner, Move loser) const {
  for (const auto &rule : rules_) {
    if (rule.winner == winner && rule.loser == loser) {
      return toString(winner) + " " + rule.verb + " " + toString(loser);
    }
  }
  return toString(winner) + " beats " + toString(loser);
}

GameRules GameRules::standard() {
  GameRules rules;
  rules.addRule(Move::Paper, Move::Rock, "wraps");
  rules.addRule(Move::Rock, Move::Scissors, "blunts");
  rules.addRule(Move::Scissors, Move::Paper, "cuts");
  return rules;
}