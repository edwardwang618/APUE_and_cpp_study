#pragma once

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Socket {
  int fd_ = -1;

public:
  Socket() = default;

  explicit Socket(int fd) : fd_(fd) {
    if (fd_ < 0)
      throw std::runtime_error("Invalid socket");
  }

  ~Socket() {
    if (fd_ >= 0)
      close(fd_);
  }

  // Non-copyable
  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  // Movable
  Socket(Socket &&other) noexcept : fd_(other.fd_) { other.fd_ = -1; }

  Socket &operator=(Socket &&other) noexcept {
    if (this != &other) {
      if (fd_ >= 0)
        close(fd_);
      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }

  int get() const { return fd_; }

  int release() {
    int fd = fd_;
    fd_ = -1;
    return fd;
  }

  explicit operator bool() const { return fd_ >= 0; }
};

class TcpSocket : public Socket {
public:
  TcpSocket() : Socket(socket(AF_INET, SOCK_STREAM, 0)) {}
  explicit TcpSocket(int fd) : Socket(fd) {}

  void bind(const std::string &ip, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (ip.empty() || ip == "0.0.0.0") {
      addr.sin_addr.s_addr = INADDR_ANY;
    } else {
      if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address: " + ip);
      }
    }

    if (::bind(get(), reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
      throw std::runtime_error("bind failed: " + std::string(strerror(errno)));
    }
  }

  void listen(int backlog = 5) {
    if (::listen(get(), backlog) < 0) {
      throw std::runtime_error("listen failed: " +
                               std::string(strerror(errno)));
    }
  }

  TcpSocket accept() {
    int connfd = ::accept(get(), nullptr, nullptr);
    if (connfd < 0) {
      throw std::runtime_error("accept failed: " +
                               std::string(strerror(errno)));
    }
    return TcpSocket(connfd);
  }

  void connect(const std::string &ip, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
      throw std::runtime_error("Invalid IP address: " + ip);
    }

    if (::connect(get(), reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) <
        0) {
      throw std::runtime_error("connect failed: " +
                               std::string(strerror(errno)));
    }
  }

  void send(const std::string &data) {
    ssize_t n = ::send(get(), data.data(), data.size(), 0);
    if (n < 0) {
      throw std::runtime_error("send failed: " + std::string(strerror(errno)));
    }
  }

  std::string recv(size_t max_size = 1024) {
    std::string buf(max_size, '\0');
    ssize_t n = ::recv(get(), buf.data(), buf.size(), 0);
    if (n < 0) {
      throw std::runtime_error("recv failed: " + std::string(strerror(errno)));
    }
    buf.resize(n);
    return buf;
  }
};