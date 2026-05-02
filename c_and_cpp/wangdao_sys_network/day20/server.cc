#include "socket.hpp"
#include <iostream>

int main() {
  try {
    TcpSocket server;
    server.bind("0.0.0.0", 1234);
    server.listen();
    std::cout << "Server listening on port 1234...\n";

    TcpSocket client = server.accept();
    std::cout << "Client connected!\n";

    std::string msg = client.recv();
    std::cout << "Received: " << msg << "\n";

    client.send("Hello Client!");

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}