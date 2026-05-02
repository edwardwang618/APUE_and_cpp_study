#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int fd[2];
  pipe(fd);
  // fd[0] = read
  // fd[1] = write

  pid_t pid = fork();

  if (pid > 0) {
    // Parent
    close(fd[0]);

    std::string msg = "hello from parent";
    write(fd[1], msg.c_str(), msg.size());

    close(fd[1]);
    wait(nullptr);
  } else {
    // Child
    close(fd[1]);

    char buf[100] = {0};
    read(fd[0], buf, sizeof(buf));
    std::cout << "Child received: " << buf << std::endl;

    close(fd[0]);
  }

  return 0;
}