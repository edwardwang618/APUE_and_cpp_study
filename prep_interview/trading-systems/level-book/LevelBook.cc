#include <cstdint>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <stdexcept>
#include <unordered_map>

// ----- Type Definitions -----

enum class Side { BUY, SELL };

struct Order {
  Side side;
  uint64_t orderId;
  uint64_t price;
  uint64_t quantity;
};

// A price level holds the aggregate quantity and a FIFO queue of orders.
// New orders are pushed to the front; matching consumes from the back (oldest
// first).
struct Level {
  uint64_t totalQuantity;
  std::list<Order> orders;
  Level() : totalQuantity(0) {}
};

// OrderHash allows O(1) lookup from orderId to the exact position in the book.
// It stores a pointer to the Level and an iterator into that Level's order
// list. IMPORTANT: This design relies on two stability guarantees:
//   1. std::map does not invalidate pointers/references to elements on
//   insert/erase of OTHER elements.
//   2. std::list does not invalidate iterators on insert/erase of OTHER
//   elements.
struct OrderLocation {
  Level *level;
  uint64_t
      price; // stored separately to locate the level in the map for cleanup
  std::list<Order>::iterator
      it; // stable as long as this specific node is not erased
};

// ----- Order Book -----

class OrderBook {
public:
  // === Add a new limit order to the book ===
  // Returns the quantity matched. Any remaining quantity rests in the book.
  uint64_t AddOrder(Order order) {
    // Reject duplicate order IDs
    if (orderMap_.find(order.orderId) != orderMap_.end()) {
      return 0;
    }

    // Step 1: Attempt to match the incoming order against the opposite side
    uint64_t matched = Match(order);

    // Step 2: If there is remaining quantity, insert it into the resting book
    if (order.quantity > 0) {
      InsertIntoBook(order);
    }

    return matched;
  }

  // === Cancel an existing order by ID ===
  // Returns true if the order was found and removed.
  bool CancelOrder(uint64_t orderId) {
    auto it = orderMap_.find(orderId);
    if (it == orderMap_.end()) {
      return false;
    }

    // Retrieve the location info BEFORE erasing from orderMap
    OrderLocation loc = it->second;
    Order order = *loc.it;

    // Update the level: decrement total quantity, remove the order node
    loc.level->totalQuantity -= order.quantity;
    loc.level->orders.erase(loc.it);

    // Remove from the orderId lookup map
    orderMap_.erase(it);

    // If this was the last order at this price level, remove the level
    // entirely. This keeps the book clean and ensures GetBestPrice() is
    // correct.
    if (loc.level->totalQuantity == 0) {
      if (order.side == Side::BUY) {
        buyBook_.erase(order.price);
      } else {
        sellBook_.erase(order.price);
      }
    }

    return true;
  }

  // === Query the best bid (highest buy price) ===
  uint64_t GetBestBid() const {
    if (buyBook_.empty()) {
      throw std::runtime_error("buy book is empty");
    }
    return buyBook_.begin()->first;
  }

  // === Query the best ask (lowest sell price) ===
  uint64_t GetBestAsk() const {
    if (sellBook_.empty()) {
      throw std::runtime_error("sell book is empty");
    }
    return sellBook_.begin()->first;
  }

  // === Query total resting quantity at a given price and side ===
  uint64_t GetQuantityAtLevel(Side side, uint64_t price) const {
    const auto &book = (side == Side::BUY) ? buyBook_ : sellBook_;
    auto it = book.find(price);
    if (it == book.end()) {
      return 0;
    }
    return it->second.totalQuantity;
  }

private:
  // Buy book: sorted descending by price (highest = best bid = begin)
  std::map<uint64_t, Level, std::greater<>> buyBook_;

  // Sell book: sorted ascending by price (lowest = best ask = begin)
  std::map<uint64_t, Level> sellBook_;

  // Global order lookup: orderId -> location in the book
  std::unordered_map<uint64_t, OrderLocation> orderMap_;

  // ----- Internal: Insert a resting order into the appropriate book -----
  void InsertIntoBook(const Order &order) {
    auto &book = (order.side == Side::BUY) ? buyBook_ : sellBook_;

    // operator[] default-constructs a Level if the price doesn't exist yet.
    // This is safe because Level() initializes totalQuantity to 0.
    auto &level = book[order.price];
    level.totalQuantity += order.quantity;

    // push_front: newest orders go to the front.
    // Matching consumes from the back (oldest first) => FIFO within a level.
    level.orders.push_front(order);

    // Record the position in orderMap for O(1) cancel.
    // &level is stable because std::map guarantees pointer stability.
    // level.orders.begin() is stable because std::list guarantees iterator
    // stability.
    orderMap_[order.orderId] = OrderLocation{
        .level = &level, .price = order.price, .it = level.orders.begin()};
  }

  // ----- Internal: Match an incoming order against the opposite book -----
  // Modifies order.quantity in place (decremented by the amount filled).
  // Returns the total matched quantity.
  uint64_t Match(Order &order) {
    if (order.side == Side::SELL) {
      return MatchAgainstBook(order, buyBook_);
    } else {
      return MatchAgainstBook(order, sellBook_);
    }
  }

  // Generic matching logic that works for both sides.
  // Template parameter allows it to work with both map orderings.
  //
  // The price matching condition differs by side:
  //   - Incoming SELL matches against buy levels where buyPrice >= sellPrice
  //   - Incoming BUY matches against sell levels where sellPrice <= buyPrice
  // Because both books are sorted with the "best" price at begin(),
  // and we iterate from begin(), the condition simplifies to:
  //   - For SELL matching against buyBook (descending): stop when buyPrice <
  //   sellPrice
  //   - For BUY matching against sellBook (ascending): stop when sellPrice >
  //   buyPrice
  // Both are equivalent to: the resting price is "worse" than the incoming
  // price.
  template <typename Comparator>
  uint64_t MatchAgainstBook(Order &order,
                            std::map<uint64_t, Level, Comparator> &book) {
    uint64_t totalMatched = 0;

    for (auto it = book.begin(); it != book.end() && order.quantity > 0;) {
      auto &[price, level] = *it;

      // Check if this price level can trade with the incoming order.
      // For a SELL order hitting the buy book: buy price must be >= sell price.
      // For a BUY order hitting the sell book: sell price must be <= buy price.
      if (!PricesCross(order.side, order.price, price)) {
        break; // No more matchable levels (book is sorted by attractiveness)
      }

      if (order.quantity >= level.totalQuantity) {
        // --- Optimization: sweep the entire level ---
        // When the incoming quantity exceeds the level's total,
        // we can skip per-order quantity arithmetic.
        // However, we still must iterate to clean up orderMap entries.
        totalMatched += level.totalQuantity;
        order.quantity -= level.totalQuantity;

        for (auto &restingOrder : level.orders) {
          orderMap_.erase(restingOrder.orderId);
        }

        // erase returns the next valid iterator
        it = book.erase(it);
      } else {
        // --- Partial fill of this level ---
        // Consume orders from the back (oldest first = FIFO).
        while (order.quantity > 0) {
          // IMPORTANT: Fetch the reference INSIDE the loop.
          // After pop_back(), the previous reference is dangling.
          auto &backOrder = level.orders.back();

          if (order.quantity >= backOrder.quantity) {
            // Fully fill this resting order
            totalMatched += backOrder.quantity;
            order.quantity -= backOrder.quantity;
            level.totalQuantity -= backOrder.quantity;
            orderMap_.erase(backOrder.orderId);
            level.orders.pop_back();
            // The reference 'backOrder' is now dangling.
            // The next iteration will fetch a fresh one.
          } else {
            // Partially fill this resting order
            // IMPORTANT: Update totalQuantity and backOrder.quantity
            // BEFORE zeroing order.quantity, otherwise you subtract 0.
            totalMatched += order.quantity;
            level.totalQuantity -= order.quantity;
            backOrder.quantity -= order.quantity;
            order.quantity = 0;
          }
        }
        // Don't advance the iterator; we break out since the incoming order is
        // filled.
        break;
      }
    }

    return totalMatched;
  }

  // Returns true if the incoming order price and resting price can trade.
  static bool PricesCross(Side incomingSide, uint64_t incomingPrice,
                          uint64_t restingPrice) {
    if (incomingSide == Side::SELL) {
      // Seller is willing to sell at incomingPrice or higher.
      // Buyer is willing to buy at restingPrice or lower.
      // Trade happens if buyPrice >= sellPrice.
      return restingPrice >= incomingPrice;
    } else {
      // Buyer is willing to buy at incomingPrice or lower.
      // Seller is willing to sell at restingPrice or higher.
      // Trade happens if sellPrice <= buyPrice.
      return restingPrice <= incomingPrice;
    }
  }
};