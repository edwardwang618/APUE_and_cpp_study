#include "Game.h"
#include "GameRules.h"
#include "RandomPlayer.h"

#include <cstdlib>
#include <ctime>
#include <memory>

int main() {
  std::srand(static_cast<unsigned>(std::time(nullptr)));

  auto rules = GameRules::standard();

  auto p1 = std::make_unique<RandomPlayer>("Alice");
  auto p2 = std::make_unique<RandomPlayer>("Bob");

  Game game(std::move(rules), std::move(p1), std::move(p2));
  game.playRounds(5);

  return 0;
}