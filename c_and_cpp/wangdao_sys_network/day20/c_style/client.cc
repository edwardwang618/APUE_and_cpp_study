#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <IP> <Port>\n";
    return 1;
  }

  // 1. Create socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    return 1;
  }

  // 2. Build server address
  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(std::stoi(argv[2]));
  inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

  // 3. Connect
  std::cout << "Connecting to " << argv[1] << ":" << argv[2] << "...\n";
  if (connect(sockfd, reinterpret_cast<sockaddr *>(&server_addr),
              sizeof(server_addr)) < 0) {
    perror("connect");
    return 1;
  }
  std::cout << "Connected!\n";

  // 4. Send
  std::string msg = "Hello Server!";
  send(sockfd, msg.data(), msg.size(), 0);

  // 5. Receive
  char buf[1024]{};
  recv(sockfd, buf, sizeof(buf), 0);
  std::cout << "Received: " << buf << "\n";

  close(sockfd);
  return 0;
}