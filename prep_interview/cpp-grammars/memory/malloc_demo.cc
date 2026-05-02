#include <cstdlib>
#include <iostream>

class Task {
public:
  int id;
  char *data;

  Task(int i) : id(i), data(new char[100]) {
    std::cout << "Task(" << id << ") 构造, data=" << (void *)data << "\n";
  }

  ~Task() {
    std::cout << "Task(" << id << ") 析构, 释放 data\n";
    delete[] data;
  }
};

int main() {
  std::cout << "=== malloc + free ===\n";
  // malloc 只分配内存，不调用构造函数
  Task *p1 = (Task *)malloc(sizeof(Task));
  // p1->id 和 p1->data 都是未初始化的垃圾值
  // 构造函数从未执行，new char[100] 从未发生
  free(p1); // 只释放 Task 结构体本身，无泄漏（因为 data 从未真正分配）
  std::cout << "free 完成\n";

  std::cout << "\n=== new + delete ===\n";
  // new = operator new + 构造函数
  Task *p2 = new Task(42);
  // delete = 析构函数 + operator delete
  delete p2;
  std::cout << "delete 完成\n";

  std::cout << "\n=== new + free (错误！内存泄漏) ===\n";
  Task *p3 = new Task(99); // 构造函数分配了 data
  free(p3); // 不调用析构函数，data 指向的 100 字节泄漏！
  std::cout << "free 完成，但 data 泄漏了！\n";

  return 0;
}