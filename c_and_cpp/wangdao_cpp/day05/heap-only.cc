class HeapOnly {
public:
  static HeapOnly *create() { return new HeapOnly(); }

  void destroy() { delete this; }

private:
  HeapOnly() {}  // Private: can't create on stack
  ~HeapOnly() {} // Private: can't use delete directly
};

int main() {
  // HeapOnly obj;              // ✗ ERROR: private constructor
  // HeapOnly* p = new HeapOnly();  // ✗ ERROR: private constructor

  HeapOnly *p = HeapOnly::create(); // ✓ OK
  p->destroy();                     // ✓ OK
}