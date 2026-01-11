template <typename T> class List {
public:
  List();
  ~List();

  List(const List &other);
  List(List &&other);
  List &operator=(List other);

  void push_back(const T &value);
  void push_front(const T &value);
  void pop_back();
  void pop_front();

  size_t size() const;
  bool empty() const;

  // 迭代器
  struct iterator;
  iterator begin();
  iterator end();

  iterator insert(iterator pos, const T &value);
  iterator erase(iterator pos);

  // 链表特有操作
  void splice(iterator pos, List &other); // 把 other 整个移到 pos 前面
};