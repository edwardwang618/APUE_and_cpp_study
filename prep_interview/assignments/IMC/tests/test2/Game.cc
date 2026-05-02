#include "Game.h"

#include <iostream>

Game::Game(GameRules rules, std::unique_ptr<Player> p1,
           std::unique_ptr<Player> p2)
    : rules_(std::move(rules)), p1_(std::move(p1)), p2_(std::move(p2)) {}

void Game::playRounds(int n) {
  for (int i = 1; i <= n; ++i) {
    std::cout << "--- Round " << i << " ---\n";
    playRound();
  }
  printSummary();
}

void Game::playRound() {
  Move m1 = p1_->chooseMove();
  Move m2 = p2_->chooseMove();

  std::cout << p1_->name() << " plays " << toString(m1) << "\n";
  std::cout << p2_->name() << " plays " << toString(m2) << "\n";

  GameResult result = rules_.judge(m1, m2);
  switch (result) {
  case GameResult::Win:
    std::cout << rules_.describe(m1, m2) << " — " << p1_->name()
              << " wins!\n\n";
    ++wins1_;
    break;
  case GameResult::Loss:
    std::cout << rules_.describe(m2, m1) << " — " << p2_->name()
              << " wins!\n\n";
    ++wins2_;
    break;
  case GameResult::Draw:
    std::cout << "Draw!\n\n";
    ++draws_;
    break;
  }
}

void Game::printSummary() const {
  std::cout << "=== Summary ===\n"
            << p1_->name() << ": " << wins1_ << " wins\n"
            << p2_->name() << ": " << wins2_ << " wins\n"
            << "Draws: " << draws_ << "\n";
}