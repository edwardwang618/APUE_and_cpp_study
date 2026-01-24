#include "arena.hpp"
#include "lockfree_pool.hpp"
#include "pool.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace memory;

// =============================================================================
// Example types
// =============================================================================

struct Order {
  uint64_t id;
  uint64_t price;
  uint32_t quantity;
  char side; // 'B' or 'S'

  Order() : id(0), price(0), quantity(0), side('B') {}

  Order(uint64_t id_, uint64_t price_, uint32_t qty_, char side_)
      : id(id_), price(price_), quantity(qty_), side(side_) {}

  ~Order() = default;
};

struct MarketDataUpdate {
  uint64_t timestamp;
  uint64_t bid;
  uint64_t ask;
  uint32_t bid_size;
  uint32_t ask_size;
  char symbol[16];
};

// =============================================================================
// Arena example: batch processing
// =============================================================================

void demo_arena() {
  printf("=== Arena Example ===\n");

  Arena arena(4096);

  // Allocate market data updates
  auto *update1 = arena.create<MarketDataUpdate>();
  update1->timestamp = 1234567890;
  update1->bid = 10000;
  update1->ask = 10001;
  std::strcpy(update1->symbol, "AAPL");

  auto *update2 = arena.create<MarketDataUpdate>();
  update2->timestamp = 1234567891;
  update2->bid = 10001;
  update2->ask = 10002;
  std::strcpy(update2->symbol, "AAPL");

  // Allocate array of prices
  auto *prices = arena.createArray<uint64_t>(10);
  for (int i = 0; i < 10; i++) {
    prices[i] = 10000 + i;
  }

  printf("Arena used: %zu bytes\n", arena.used());
  printf("Arena remaining: %zu bytes\n", arena.remaining());

  // Process the batch...
  printf("Processing %s: bid=%lu ask=%lu\n", update1->symbol, update1->bid,
         update1->ask);

  // Done with batch, reset for next
  arena.reset();
  printf("After reset, used: %zu bytes\n\n", arena.used());
}

// =============================================================================
// Pool example: order management
// =============================================================================

void demo_pool() {
  printf("=== Pool Example ===\n");

  Pool<Order> order_pool(1000);

  // Create orders
  Order *order1 = order_pool.create(1, 10000, 100, 'B');
  Order *order2 = order_pool.create(2, 10001, 200, 'S');
  Order *order3 = order_pool.create(3, 10002, 150, 'B');

  printf("Created orders:\n");
  printf("  Order %lu: price=%lu qty=%u side=%c\n", order1->id, order1->price,
         order1->quantity, order1->side);
  printf("  Order %lu: price=%lu qty=%u side=%c\n", order2->id, order2->price,
         order2->quantity, order2->side);
  printf("  Order %lu: price=%lu qty=%u side=%c\n", order3->id, order3->price,
         order3->quantity, order3->side);

  // Cancel order 2 (returns slot to pool)
  printf("Canceling order 2...\n");
  order_pool.destroy(order2);

  // Create new order (reuses order2's slot)
  Order *order4 = order_pool.create(4, 10003, 300, 'S');
  printf("Created order %lu (reused slot)\n\n", order4->id);

  // Cleanup
  order_pool.destroy(order1);
  order_pool.destroy(order3);
  order_pool.destroy(order4);
}

// =============================================================================
// Lock-free pool example
// =============================================================================

void demo_lockfree_pool() {
  printf("=== Lock-Free Pool Example ===\n");

  LockFreePool<Order, 1024> pool;

  Order *orders[10];

  // Allocate 10 orders
  for (int i = 0; i < 10; i++) {
    orders[i] = pool.create(i, 10000 + i, 100 * (i + 1), i % 2 ? 'B' : 'S');
  }
  printf("Created 10 orders\n");

  // Free odd-indexed orders
  for (int i = 1; i < 10; i += 2) {
    pool.destroy(orders[i]);
  }
  printf("Freed 5 orders (odd indices)\n");

  // Reallocate (reuses freed slots)
  for (int i = 1; i < 10; i += 2) {
    orders[i] = pool.create(100 + i, 20000 + i, 500, 'B');
  }
  printf("Reallocated 5 orders\n\n");

  // Cleanup
  for (int i = 0; i < 10; i++) {
    pool.destroy(orders[i]);
  }
}

// =============================================================================
// Performance comparison
// =============================================================================

void demo_performance() {
  printf("=== Performance Comparison ===\n");

  const int ITERATIONS = 1000000;
  const int ACTIVE_SLOTS = 1000;

  // Pool benchmark
  {
    Pool<Order> pool(ITERATIONS);
    Order *orders[ACTIVE_SLOTS] = {};

    auto start = __builtin_readcyclecounter();

    for (int i = 0; i < ITERATIONS; i++) {
      int idx = i % ACTIVE_SLOTS;
      if (i >= ACTIVE_SLOTS) {
        pool.free(orders[idx]); // free old
      }
      orders[idx] = pool.alloc(); // alloc new
    }

    auto end = __builtin_readcyclecounter();

    printf("Pool:   %llu cycles for %d ops (%.1f cycles/op)\n", end - start,
           ITERATIONS, (double)(end - start) / ITERATIONS);
  }

  // malloc benchmark
  {
    Order *orders[ACTIVE_SLOTS] = {};

    auto start = __builtin_readcyclecounter();

    for (int i = 0; i < ITERATIONS; i++) {
      int idx = i % ACTIVE_SLOTS;
      if (i >= ACTIVE_SLOTS) {
        std::free(orders[idx]);
      }
      orders[idx] = static_cast<Order *>(std::malloc(sizeof(Order)));
    }

    auto end = __builtin_readcyclecounter();

    printf("malloc: %llu cycles for %d ops (%.1f cycles/op)\n", end - start,
           ITERATIONS, (double)(end - start) / ITERATIONS);

    // Cleanup
    for (int i = 0; i < ACTIVE_SLOTS; i++) {
      std::free(orders[i]);
    }
  }

  printf("\n");
}

// =============================================================================
// Main
// =============================================================================

int main() {
  demo_arena();
  demo_pool();
  demo_lockfree_pool();
  demo_performance();

  return 0;
}