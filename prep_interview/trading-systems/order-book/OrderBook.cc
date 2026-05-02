#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <new>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

enum class Side { BUY, SELL };

struct Order {
  Side side_;
  uint64_t order_id_;
  int price_;
  uint32_t quantity_;
  Order *prev_ = nullptr, *next_ = nullptr;
  Order() = default;
  Order(Side side, uint64_t order_id, int price, uint32_t quantity)
      : side_(side), order_id_(order_id), price_(price), quantity_(quantity) {}
};

struct Allocator {
  std::vector<Order *> pools_;
  Order *free_head_ = nullptr;
  size_t chunksize_ = 16;

  void Extend() {
    free_head_ =
        static_cast<Order *>(::operator new(chunksize_ * sizeof(Order)));
    pools_.push_back(free_head_);
    for (size_t i = 0; i < chunksize_; i++) {
      free_head_[i].next_ = i + 1 < chunksize_ ? &free_head_[i + 1] : nullptr;
    }
    chunksize_ = std::min(chunksize_ * 2, static_cast<size_t>(1 << 20));
  }

public:
  Allocator() { Extend(); }
  ~Allocator() {
    for (Order *p : pools_) {
      ::operator delete(p);
    }
  }
  Order *Allocate() {
    if (!free_head_) {
      Extend();
    }
    Order *order = free_head_;
    free_head_ = free_head_->next_;
    return order;
  }
  void Deallocate(Order *order) {
    order->next_ = free_head_;
    free_head_ = order;
  }
};

Allocator allocator;

struct Level {
  Order *head_, *tail_;
  uint32_t total_quantity_;
  Level() : head_(nullptr), tail_(nullptr), total_quantity_(0) {}
  Order *AddOrder(Side side, uint64_t order_id, int price, uint32_t quantity) {
    Order *order =
        new (allocator.Allocate()) Order(side, order_id, price, quantity);
    if (!head_) {
      head_ = tail_ = order;
    } else {
      order->next_ = head_;
      head_->prev_ = order;
      head_ = order;
    }
    total_quantity_ += quantity;
    return order;
  }

  void RemoveOrder(Order *order) {
    if (order == head_) {
      head_ = head_->next_;
    }
    if (order == tail_) {
      tail_ = tail_->prev_;
    }
    if (order->prev_) {
      order->prev_->next_ = order->next_;
    }
    if (order->next_) {
      order->next_->prev_ = order->prev_;
    }
    total_quantity_ -= order->quantity_;
    allocator.Deallocate(order);
  }
};

class OrderBook {
  std::map<int, Level, std::greater<>> buy_book_;
  std::map<int, Level> sell_book_;
  std::unordered_map<uint64_t, Order *> order_map_;

  uint32_t MatchOrder(Order order, auto &book) {
    uint32_t matched_quantity = 0;
    uint32_t remaining_quantity = order.quantity_;
    for (auto it = book.begin(); it != book.end();) {
      if (order.side_ == Side::BUY && order.price_ < it->first) {
        break;
      }
      if (order.side_ == Side::SELL && order.price_ > it->first) {
        break;
      }
      if (remaining_quantity == 0) {
        break;
      }
      auto &level = it->second;
      if (remaining_quantity >= level.total_quantity_) {
        remaining_quantity -= level.total_quantity_;
        matched_quantity += level.total_quantity_;
        Order *cur = level.head_;
        while (cur) {
          order_map_.erase(cur->order_id_);
          Order *next = cur->next_;
          level.RemoveOrder(cur);
          cur = next;
        }
        it = book.erase(it);
      } else {
        Order *cur = level.tail_;
        while (remaining_quantity > 0) {
          if (remaining_quantity >= cur->quantity_) {
            remaining_quantity -= cur->quantity_;
            matched_quantity += cur->quantity_;
            order_map_.erase(cur->order_id_);
            Order *prev = cur->prev_;
            level.RemoveOrder(cur);
            cur = prev;
          } else {
            cur->quantity_ -= remaining_quantity;
            level.total_quantity_ -= remaining_quantity;
            matched_quantity += remaining_quantity;
            remaining_quantity = 0;
          }
        }
      }
    }
    if (remaining_quantity > 0) {
      Order *new_order = nullptr;
      if (order.side_ == Side::BUY) {
        new_order = buy_book_[order.price_].AddOrder(
            Side::BUY, order.order_id_, order.price_, remaining_quantity);
      } else {
        new_order = sell_book_[order.price_].AddOrder(
            Side::SELL, order.order_id_, order.price_, remaining_quantity);
      }
      order_map_[order.order_id_] = new_order;
    }
    return matched_quantity;
  }

  uint32_t MatchOrderAgainstBuy(Order order) {
    assert(order.side_ == Side::SELL);
    return MatchOrder(order, buy_book_);
  }
  uint32_t MatchOrderAgainsSell(Order order) {
    assert(order.side_ == Side::BUY);
    return MatchOrder(order, sell_book_);
  }

  std::pair<double, uint32_t> GetVWAP(uint32_t target_quantity,
                                      const auto &book) const {
    if (book.empty() || target_quantity == 0) {
      return std::make_pair(0.0, 0);
    }
    double sum = 0.0;
    uint32_t filled = 0;
    for (auto it = book.begin(); it != book.end() && target_quantity > 0;
         ++it) {
      auto &level = it->second;
      if (target_quantity >= level.total_quantity_) {
        filled += level.total_quantity_;
        sum += (double)it->first * level.total_quantity_;
        target_quantity -= level.total_quantity_;
      } else {
        filled += target_quantity;
        sum += (double)it->first * target_quantity;
        target_quantity = 0;
      }
    }
    return std::make_pair(sum / filled, filled);
  }

public:
  uint32_t AddOrder(Order order) {
    if (order_map_.contains(order.order_id_)) {
      return 0;
    }
    if (order.side_ == Side::BUY) {
      return MatchOrderAgainsSell(order);
    } else {
      return MatchOrderAgainstBuy(order);
    }
  }

  bool CancelOrder(uint64_t order_id) {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) {
      return false;
    }
    Order *order = it->second;
    Side side = order->side_;
    int price = order->price_;
    if (side == Side::BUY) {
      assert(buy_book_.find(price) != buy_book_.end());
    } else {
      assert(sell_book_.find(price) != sell_book_.end());
    }
    auto &level = side == Side::BUY ? buy_book_[price] : sell_book_[price];
    level.RemoveOrder(order);
    if (level.total_quantity_ == 0) {
      if (side == Side::BUY) {
        buy_book_.erase(price);
      } else {
        sell_book_.erase(price);
      }
    }
    order_map_.erase(it);
    return true;
  }

  bool ModifyOrder(uint64_t order_id, uint32_t new_quantity) {
    auto it = order_map_.find(order_id);
    if (it == order_map_.end()) {
      return false;
    }

    Order *order = it->second;
    Side side = order->side_;
    int price = order->price_;
    auto &level = side == Side::BUY ? buy_book_[price] : sell_book_[price];
    if (new_quantity == 0) {
      CancelOrder(order_id);
      return true;
    }
    if (new_quantity <= order->quantity_) {
      level.total_quantity_ -= order->quantity_ - new_quantity;
      order->quantity_ = new_quantity;
    } else {
      order_map_[order_id] =
          level.AddOrder(side, order->order_id_, price, new_quantity);
      level.RemoveOrder(order);
    }
    return true;
  }

  std::optional<std::pair<int, uint32_t>> GetBestBid() const {
    if (buy_book_.empty()) {
      return std::nullopt;
    }
    auto it = buy_book_.begin();
    return std::make_pair(it->first, it->second.total_quantity_);
  }
  std::optional<std::pair<int, uint32_t>> GetBestAsk() const {
    if (sell_book_.empty()) {
      return std::nullopt;
    }
    auto it = sell_book_.begin();
    return std::make_pair(it->first, it->second.total_quantity_);
  }

  const Level &GetLevel(Side side, int depth) const {
    assert(depth >= 0);
    if (side == Side::BUY) {
      assert(depth < buy_book_.size());
      auto it = buy_book_.begin();
      while (depth--) {
        it = std::next(it);
      }
      return it->second;
    } else {
      assert(depth < sell_book_.size());
      auto it = sell_book_.begin();
      while (depth--) {
        it = std::next(it);
      }
      return it->second;
    }
  }

  std::optional<double> GetMid() const {
    if (buy_book_.empty() || sell_book_.empty()) {
      return std::nullopt;
    }
    return (buy_book_.begin()->first + sell_book_.begin()->first) / 2.0;
  }

  std::optional<int> GetSpread() const {
    if (buy_book_.empty() || sell_book_.empty()) {
      return std::nullopt;
    }
    return sell_book_.begin()->first - buy_book_.begin()->first;
  }

  std::pair<double, uint32_t> GetVWAP(Side side,
                                      uint32_t target_quantity) const {
    if (side == Side::BUY) {
      return GetVWAP(target_quantity, buy_book_);
    } else {
      return GetVWAP(target_quantity, sell_book_);
    }
  }
};