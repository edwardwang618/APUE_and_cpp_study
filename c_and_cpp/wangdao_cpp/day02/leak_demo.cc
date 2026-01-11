int main() {
  int *p = new int;
  int *q = new int;
  p = q;
  delete q;
}