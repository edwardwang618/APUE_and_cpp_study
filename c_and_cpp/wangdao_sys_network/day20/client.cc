#include "socket.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <IP> <Port>\n";
    return 1;
  }

  try {
    TcpSocket sock;
    sock.connect(argv[1], std::stoi(argv[2]));
    std::cout << "Connected!\n";

    sock.send("Hello Server!");

    std::string reply = sock.recv();
    std::cout << "Received: " << reply << "\n";

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}