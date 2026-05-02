#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  // 1. Create socket
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    perror("socket");
    return 1;
  }

  // 2. Bind
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(listenfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  // 3. Listen
  if (listen(listenfd, 5) < 0) {
    perror("listen");
    return 1;
  }
  std::cout << "Server listening on port 1234...\n";

  // 4. Accept
  int connfd = accept(listenfd, nullptr, nullptr);
  if (connfd < 0) {
    perror("accept");
    return 1;
  }
  std::cout << "Client connected!\n";

  // 5. Recv/Send
  char buf[1024]{};
  ssize_t n = recv(connfd, buf, sizeof(buf), 0);
  if (n < 0) {
    perror("recv");
    return 1;
  }
  std::cout << "Received: " << buf << "\n";

  std::string reply = "Hello Client!";
  if (send(connfd, reply.data(), reply.size(), 0) < 0) {
    perror("send");
    return 1;
  }

  close(connfd);
  close(listenfd);
  return 0;
}