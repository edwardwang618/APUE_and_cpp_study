#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int PORT = 8080;
constexpr int MAX_EVENTS = 1024;
constexpr int BUFFER_SIZE = 4096;

// ============================================================
// Utility Functions
// 工具函数
// ============================================================

// Set file descriptor to non-blocking mode
// 设置文件描述符为非阻塞模式
bool set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    std::cerr << "fcntl F_GETFL failed: " << strerror(errno) << "\n";
    return false;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    std::cerr << "fcntl F_SETFL failed: " << strerror(errno) << "\n";
    return false;
  }
  return true;
}

// ============================================================
// FileDescriptor - RAII wrapper for file descriptors
// FileDescriptor - 文件描述符的 RAII 封装
// ============================================================

class FileDescriptor {
public:
  FileDescriptor() : fd_(-1) {}

  explicit FileDescriptor(int fd) : fd_(fd) {}

  // Move constructor
  // 移动构造函数
  FileDescriptor(FileDescriptor &&other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
  }

  // Move assignment
  // 移动赋值
  FileDescriptor &operator=(FileDescriptor &&other) noexcept {
    if (this != &other) {
      close();
      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }

  // Disable copy
  // 禁用拷贝
  FileDescriptor(const FileDescriptor &) = delete;
  FileDescriptor &operator=(const FileDescriptor &) = delete;

  ~FileDescriptor() { close(); }

  void close() {
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
  }

  int get() const { return fd_; }
  bool valid() const { return fd_ >= 0; }

  // Release ownership (caller takes responsibility)
  // 释放所有权（调用者负责关闭）
  int release() {
    int fd = fd_;
    fd_ = -1;
    return fd;
  }

private:
  int fd_;
};

// ============================================================
// Connection - Represents a client connection
// Connection - 表示一个客户端连接
// ============================================================

class Connection {
public:
  explicit Connection(int fd, const std::string &addr, int port)
      : fd_(fd), address_(addr), port_(port) {}

  int fd() const { return fd_.get(); }
  const std::string &address() const { return address_; }
  int port() const { return port_; }

  // Read data from connection
  // 从连接读取数据
  // Returns: bytes read, 0 for closed, -1 for EAGAIN, -2 for error
  // 返回: 读取字节数, 0 表示关闭, -1 表示 EAGAIN, -2 表示错误
  ssize_t read(std::string &data) {
    char buffer[BUFFER_SIZE];
    ssize_t n = ::read(fd_.get(), buffer, sizeof(buffer));

    if (n > 0) {
      data.append(buffer, n);
      return n;
    } else if (n == 0) {
      // Connection closed by peer
      // 对端关闭连接
      return 0;
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No more data available (non-blocking)
        // 没有更多数据可读（非阻塞）
        return -1;
      }
      // Actual error
      // 实际错误
      return -2;
    }
  }

  // Write data to connection
  // 向连接写入数据
  ssize_t write(const std::string &data) {
    return ::write(fd_.get(), data.c_str(), data.size());
  }

private:
  FileDescriptor fd_;
  std::string address_;
  int port_;
};

// ============================================================
// Epoll - Wrapper for epoll operations
// Epoll - epoll 操作的封装
// ============================================================

class Epoll {
public:
  Epoll() : epfd_(-1) {}

  ~Epoll() = default;

  // Initialize epoll instance
  // 初始化 epoll 实例
  bool init() {
    // epoll_create1(0): create epoll instance
    // EPOLL_CLOEXEC: close fd on exec() (optional flag)
    // epoll_create1(0): 创建 epoll 实例
    // EPOLL_CLOEXEC: exec() 时自动关闭（可选标志）
    int fd = epoll_create1(0);
    if (fd == -1) {
      std::cerr << "epoll_create1 failed: " << strerror(errno) << "\n";
      return false;
    }
    epfd_ = FileDescriptor(fd);
    return true;
  }

  // Add fd to epoll with specified events
  // 将 fd 添加到 epoll，监控指定事件
  bool add(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;

    // EPOLL_CTL_ADD: register new fd
    // EPOLL_CTL_ADD: 注册新的 fd
    if (epoll_ctl(epfd_.get(), EPOLL_CTL_ADD, fd, &ev) == -1) {
      std::cerr << "epoll_ctl ADD failed: " << strerror(errno) << "\n";
      return false;
    }
    return true;
  }

  // Modify events for existing fd
  // 修改已注册 fd 的事件
  bool modify(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;

    // EPOLL_CTL_MOD: modify existing fd
    // EPOLL_CTL_MOD: 修改已有的 fd
    if (epoll_ctl(epfd_.get(), EPOLL_CTL_MOD, fd, &ev) == -1) {
      std::cerr << "epoll_ctl MOD failed: " << strerror(errno) << "\n";
      return false;
    }
    return true;
  }

  // Remove fd from epoll
  // 从 epoll 中移除 fd
  bool remove(int fd) {
    // EPOLL_CTL_DEL: remove fd from epoll
    // event parameter can be nullptr for DEL operation
    // EPOLL_CTL_DEL: 从 epoll 中移除 fd
    // DEL 操作时 event 参数可以为 nullptr
    if (epoll_ctl(epfd_.get(), EPOLL_CTL_DEL, fd, nullptr) == -1) {
      std::cerr << "epoll_ctl DEL failed: " << strerror(errno) << "\n";
      return false;
    }
    return true;
  }

  // Wait for events
  // timeout: -1 = block forever, 0 = return immediately, >0 = wait ms
  // 等待事件
  // timeout: -1 = 永久阻塞, 0 = 立即返回, >0 = 等待毫秒数
  int wait(std::vector<epoll_event> &events, int timeout = -1) {
    // epoll_wait blocks until:
    // 1. At least one fd is ready
    // 2. Timeout expires
    // 3. Interrupted by signal
    // epoll_wait 阻塞直到:
    // 1. 至少一个 fd 就绪
    // 2. 超时
    // 3. 被信号中断
    int n = epoll_wait(epfd_.get(), events.data(), events.size(), timeout);
    if (n == -1) {
      if (errno == EINTR) {
        // Interrupted by signal, not an error
        // 被信号中断，不是错误
        return 0;
      }
      std::cerr << "epoll_wait failed: " << strerror(errno) << "\n";
    }
    return n;
  }

private:
  FileDescriptor epfd_;
};

// ============================================================
// TcpServer - Main server class using epoll
// TcpServer - 使用 epoll 的主服务器类
// ============================================================

class TcpServer {
public:
  TcpServer(int port) : port_(port), running_(false) {}

  ~TcpServer() { stop(); }

  // Initialize server
  // 初始化服务器
  bool init() {
    // Create listening socket
    // 创建监听套接字
    if (!create_listen_socket()) {
      return false;
    }

    // Initialize epoll
    // 初始化 epoll
    if (!epoll_.init()) {
      return false;
    }

    // Add listening socket to epoll
    // EPOLLIN: notify when new connection arrives
    // 将监听套接字添加到 epoll
    // EPOLLIN: 有新连接到达时通知
    if (!epoll_.add(listen_fd_.get(), EPOLLIN)) {
      return false;
    }

    std::cout << "[Server] Listening on port " << port_ << "\n";
    return true;
  }

  // Main event loop
  // 主事件循环
  void run() {
    running_ = true;
    std::vector<epoll_event> events(MAX_EVENTS);

    while (running_) {
      // Wait for events (block indefinitely)
      // 等待事件（无限期阻塞）
      int nready = epoll_.wait(events, -1);

      if (nready < 0) {
        break;
      }

      // Process all ready events
      // 处理所有就绪事件
      for (int i = 0; i < nready; ++i) {
        int fd = events[i].data.fd;
        uint32_t ev = events[i].events;

        if (fd == listen_fd_.get()) {
          // New connection on listening socket
          // 监听套接字有新连接
          handle_accept();
        } else {
          // Event on client socket
          // 客户端套接字有事件
          handle_client_event(fd, ev);
        }
      }
    }
  }

  void stop() { running_ = false; }

private:
  // Create and configure listening socket
  // 创建并配置监听套接字
  bool create_listen_socket() {
    // Create TCP socket
    // AF_INET: IPv4
    // SOCK_STREAM: TCP
    // 创建 TCP 套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      std::cerr << "socket failed: " << strerror(errno) << "\n";
      return false;
    }
    listen_fd_ = FileDescriptor(fd);

    // Set SO_REUSEADDR to avoid "Address already in use"
    // 设置 SO_REUSEADDR 避免 "Address already in use"
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      std::cerr << "setsockopt failed: " << strerror(errno) << "\n";
      return false;
    }

    // Configure server address
    // 配置服务器地址
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    addr.sin_port = htons(port_);      // Host to network byte order

    // Bind socket to address
    // 绑定套接字到地址
    if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1) {
      std::cerr << "bind failed: " << strerror(errno) << "\n";
      return false;
    }

    // Start listening
    // SOMAXCONN: system maximum backlog
    // 开始监听
    // SOMAXCONN: 系统最大积压队列长度
    if (listen(fd, SOMAXCONN) == -1) {
      std::cerr << "listen failed: " << strerror(errno) << "\n";
      return false;
    }

    // Set non-blocking for edge-triggered epoll
    // 设置非阻塞用于边缘触发 epoll
    if (!set_nonblocking(fd)) {
      return false;
    }

    return true;
  }

  // Accept new connections
  // 接受新连接
  void handle_accept() {
    // Loop to accept all pending connections (edge-triggered mode)
    // 循环接受所有等待的连接（边缘触发模式）
    while (true) {
      sockaddr_in client_addr{};
      socklen_t client_len = sizeof(client_addr);

      // Accept new connection
      // 接受新连接
      int client_fd =
          accept(listen_fd_.get(), reinterpret_cast<sockaddr *>(&client_addr),
                 &client_len);

      if (client_fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          // No more pending connections
          // 没有更多等待的连接
          break;
        }
        std::cerr << "accept failed: " << strerror(errno) << "\n";
        break;
      }

      // Get client info
      // 获取客户端信息
      std::string client_ip = inet_ntoa(client_addr.sin_addr);
      int client_port = ntohs(client_addr.sin_port);

      std::cout << "[+] New connection: fd=" << client_fd
                << ", ip=" << client_ip << ", port=" << client_port << "\n";

      // Set non-blocking
      // 设置非阻塞
      if (!set_nonblocking(client_fd)) {
        close(client_fd);
        continue;
      }

      // Add to epoll with edge-triggered mode
      // EPOLLIN: monitor for readable
      // EPOLLET: edge-triggered (only notify on state change)
      // EPOLLRDHUP: notify when peer closes connection
      // 添加到 epoll，使用边缘触发模式
      // EPOLLIN: 监控可读
      // EPOLLET: 边缘触发（仅在状态变化时通知）
      // EPOLLRDHUP: 对端关闭时通知
      if (!epoll_.add(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP)) {
        close(client_fd);
        continue;
      }

      // Store connection
      // 保存连接
      connections_.emplace(client_fd, std::make_unique<Connection>(
                                          client_fd, client_ip, client_port));
    }
  }

  // Handle events on client socket
  // 处理客户端套接字上的事件
  void handle_client_event(int fd, uint32_t events) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) {
      return;
    }

    Connection *conn = it->second.get();

    // Check for errors or hangup
    // EPOLLERR: error on fd
    // EPOLLHUP: hang up (connection closed)
    // EPOLLRDHUP: peer closed connection
    // 检查错误或挂起
    // EPOLLERR: fd 上有错误
    // EPOLLHUP: 挂起（连接关闭）
    // EPOLLRDHUP: 对端关闭连接
    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
      close_connection(fd);
      return;
    }

    // Handle readable event
    // 处理可读事件
    if (events & EPOLLIN) {
      handle_read(conn);
    }
  }

  // Read data from client (edge-triggered: must read all available data)
  // 从客户端读取数据（边缘触发：必须读取所有可用数据）
  void handle_read(Connection *conn) {
    std::string data;

    // Edge-triggered mode: must loop until EAGAIN
    // Otherwise we may miss data!
    // 边缘触发模式：必须循环直到 EAGAIN
    // 否则可能会丢失数据！
    while (true) {
      ssize_t result = conn->read(data);

      if (result > 0) {
        // Data read successfully, continue reading
        // 成功读取数据，继续读取
        continue;
      } else if (result == 0) {
        // Connection closed by peer
        // 对端关闭连接
        close_connection(conn->fd());
        return;
      } else if (result == -1) {
        // EAGAIN: no more data available
        // EAGAIN: 没有更多数据
        break;
      } else {
        // Error occurred
        // 发生错误
        close_connection(conn->fd());
        return;
      }
    }

    // Echo data back to client
    // 将数据回显给客户端
    if (!data.empty()) {
      std::cout << "[fd=" << conn->fd() << "] Received " << data.size()
                << " bytes\n";
      conn->write(data);
    }
  }

  // Close and cleanup connection
  // 关闭并清理连接
  void close_connection(int fd) {
    auto it = connections_.find(fd);
    if (it != connections_.end()) {
      std::cout << "[-] Connection closed: fd=" << fd << "\n";

      // Remove from epoll first
      // 先从 epoll 中移除
      epoll_.remove(fd);

      // Remove from connections map (destructor closes fd)
      // 从连接表中移除（析构函数关闭 fd）
      connections_.erase(it);
    }
  }

private:
  int port_;
  bool running_;
  FileDescriptor listen_fd_;
  Epoll epoll_;

  // Map fd -> Connection
  // fd 到 Connection 的映射
  std::unordered_map<int, std::unique_ptr<Connection>> connections_;
};

// ============================================================
// Main
// ============================================================

int main(int argc, char *argv[]) {
  int port = PORT;

  if (argc > 1) {
    port = std::stoi(argv[1]);
  }

  TcpServer server(port);

  if (!server.init()) {
    std::cerr << "Failed to initialize server\n";
    return 1;
  }

  std::cout << "Echo server started. Press Ctrl+C to stop.\n";
  std::cout << "Test with: nc localhost " << port << "\n";

  server.run();

  return 0;
}