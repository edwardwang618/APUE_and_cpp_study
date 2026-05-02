#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

struct Record {
  int id;
  std::string text;       // original document
  std::vector<float> vec; // embedding
};

class VectorDB {
  std::vector<Record> store;

  // Cosine similarity between two vectors
  static float cosine(const std::vector<float> &a,
                      const std::vector<float> &b) {
    float dot = 0, na = 0, nb = 0;
    for (size_t i = 0; i < a.size(); i++) {
      dot += a[i] * b[i];
      na += a[i] * a[i];
      nb += b[i] * b[i];
    }
    return dot / (std::sqrt(na) * std::sqrt(nb) + 1e-9f);
  }

public:
  // Insert a new vector
  void insert(int id, const std::string &text, const std::vector<float> &vec) {
    store.push_back({id, text, vec});
  }

  // Find top-k most similar vectors to query
  std::vector<Record> search(const std::vector<float> &query, int k) {
    // Min-heap of (score, index), keep top k
    using P = std::pair<float, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> heap;

    for (size_t i = 0; i < store.size(); i++) {
      float score = cosine(query, store[i].vec);
      if ((int)heap.size() < k)
        heap.push({score, (int)i});
      else if (score > heap.top().first) {
        heap.pop();
        heap.push({score, (int)i});
      }
    }

    std::vector<Record> results;
    while (!heap.empty()) {
      results.push_back(store[heap.top().second]);
      heap.pop();
    }
    std::reverse(results.begin(), results.end()); // best first
    return results;
  }
};

int main() {
  VectorDB db;
  db.insert(1, "cat", {1.0f, 0.2f, 0.1f});
  db.insert(2, "dog", {0.9f, 0.3f, 0.2f});
  db.insert(3, "car", {0.1f, 0.9f, 0.8f});
  db.insert(4, "truck", {0.2f, 0.8f, 0.9f});

  std::vector<float> query = {1.0f, 0.25f, 0.15f}; // something "animal-like"
  auto results = db.search(query, 2);

  std::cout << "Top matches:\n";
  for (auto &r : results)
    std::cout << "  id=" << r.id << " text=" << r.text << "\n";
}