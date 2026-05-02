#include "GameRules.h"
#include "RandomPlayer.h"

#include <cassert>
#include <iostream>
#include <set>

int main() {
  auto rules = GameRules::standard();

  // --- Draws ---
  assert(rules.judge(Move::Rock, Move::Rock) == GameResult::Draw);
  assert(rules.judge(Move::Paper, Move::Paper) == GameResult::Draw);
  assert(rules.judge(Move::Scissors, Move::Scissors) == GameResult::Draw);

  // --- Paper wraps Rock ---
  assert(rules.judge(Move::Paper, Move::Rock) == GameResult::Win);
  assert(rules.judge(Move::Rock, Move::Paper) == GameResult::Loss);

  // --- Rock blunts Scissors ---
  assert(rules.judge(Move::Rock, Move::Scissors) == GameResult::Win);
  assert(rules.judge(Move::Scissors, Move::Rock) == GameResult::Loss);

  // --- Scissors cuts Paper ---
  assert(rules.judge(Move::Scissors, Move::Paper) == GameResult::Win);
  assert(rules.judge(Move::Paper, Move::Scissors) == GameResult::Loss);

  // --- Descriptions ---
  assert(rules.describe(Move::Paper, Move::Rock) == "Paper wraps Rock");
  assert(rules.describe(Move::Rock, Move::Scissors) == "Rock blunts Scissors");
  assert(rules.describe(Move::Scissors, Move::Paper) == "Scissors cuts Paper");

  // --- RandomPlayer produces all three moves ---
  RandomPlayer rp("Test");
  std::set<Move> seen;
  for (int i = 0; i < 100; ++i) {
    Move m = rp.chooseMove();
    assert(m == Move::Rock || m == Move::Paper || m == Move::Scissors);
    seen.insert(m);
  }
  assert(seen.size() == 3);

  std::cout << "All tests passed.\n";
  return 0;
}