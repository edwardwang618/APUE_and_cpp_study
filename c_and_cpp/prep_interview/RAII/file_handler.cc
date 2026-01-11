#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>

class FileHandler {
  FILE *fp;

public:
  FileHandler(const char *path, const char *mode) : fp(fopen(path, mode)) {
    if (!fp)
      throw std::runtime_error(std::string("Cannot open file") + path);
  }

  ~FileHandler() {
    if (fp)
      fclose(fp);
  }

  FileHandler(const FileHandler &) = delete;
  FileHandler &operator=(const FileHandler &) = delete;

  FileHandler(FileHandler &&other) noexcept : fp(other.fp) {
    other.fp = nullptr;
  }

  FileHandler &operator=(FileHandler &&other) noexcept {
    if (this != &other) {
      if (fp)
        fclose(fp);
      fp = other.fp;
      other.fp = nullptr;
    }
    return *this;
  }

  FILE *get() { return fp; }
};

int main() {
  auto handler = FileHandler("data.txt", "r");
  FILE *fp = handler.get();
  return 0;
}