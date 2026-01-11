#include <algorithm>
#include <cstring>

class SSOString {
  static constexpr size_t SSO_MAX = 15;

  union {
    struct {
      char *ptr;
      size_t size;
      size_t cap;
    } heap;

    struct {
      char data[SSO_MAX + 1];
    } local;
  };

  size_t len;
  bool on_heap;

public:
  SSOString() : len(0), on_heap(false) { local.data[0] = '\0'; }

  SSOString(const char *s) {
    len = strlen(s);
    if (len <= SSO_MAX) {
      on_heap = false;
      memcpy(local.data, s, len + 1);
    } else {
      on_heap = true;
      heap.cap = len + 1;
      heap.size = len;
      heap.ptr = new char[heap.cap];
      memcpy(heap.ptr, s, len + 1);
    }
  }

  ~SSOString() {
    if (on_heap) {
      delete[] heap.ptr;
    }
  }

  const char *data() const { return on_heap ? heap.ptr : local.data; }

  size_t size() const { return len; }

  bool is_sso() const { return !on_heap; }
};