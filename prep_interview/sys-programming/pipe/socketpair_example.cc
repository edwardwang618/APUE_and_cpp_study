#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int sv[2];
  socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
  // sv[0] ◄──────► sv[1]
  // Both can read AND write

  pid_t pid = fork();

  if (pid > 0) {
    // Parent
    close(sv[1]);

    // Send to child
    std::string msg = "hello from parent";
    write(sv[0], msg.c_str(), msg.size());

    // Receive from child
    char buf[100] = {0};
    read(sv[0], buf, sizeof(buf));
    std::cout << "Parent received: " << buf << std::endl;

    close(sv[0]);
    wait(nullptr);
  } else {
    // Child
    close(sv[0]);

    // Receive from parent
    char buf[100] = {0};
    read(sv[1], buf, sizeof(buf));
    std::cout << "Child received: " << buf << std::endl;

    // Send to parent
    std::string reply = "hello from child";
    write(sv[1], reply.c_str(), reply.size());

    close(sv[1]);
  }

  return 0;
}