# Test 2 — Paper Scissors Rock

## Overview

Two players simultaneously choose a move. Rules determine the winner.

## Design Decisions

- **Data-driven rules:** `GameRules` stores (winner, loser, verb) tuples.
  No if/else chains — just a table lookup.
- **Player abstraction:** `RandomPlayer` picks a move uniformly at random.
  New strategies can be added by subclassing `Player`.
- **Game** orchestrates rounds and tracks score.

## Extensibility

- **New move**: add to the `Move` enum and call
  `addRule()` for each new relationship. No other code changes.
- **New player type**: subclass `Player`.
- **New game mode:** subclass or configure `Game`.

## Build & Run

    make            # build demo + tests
    make test       # run tests
    ./psr_demo      # two random players, 5 rounds
    make clean