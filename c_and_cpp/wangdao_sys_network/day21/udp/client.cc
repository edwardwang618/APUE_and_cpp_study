// client.cpp
#include "common.h"

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

// ============================================================
// UDP Chat Client Class UDP 聊天客户端类
// ============================================================

class UDPChatClient {
private:
  int sockfd_;               // UDP Socket
  std::string serverIp_;     // 服务器 IP
  int serverPort_;           // 服务器端口
  std::string username_;     // 用户名
  bool running_;             // 运行标志
  time_t lastHeartbeatSent_; // 最后发送心跳时间

public:
  UDPChatClient(const std::string &ip, int port, const std::string &username)
      : sockfd_(-1), serverIp_(ip), serverPort_(port), username_(username),
        running_(false), lastHeartbeatSent_(0) {}

  ~UDPChatClient() { stop(); }

  // --------------------------------------------------------
  // Start client 启动客户端
  // --------------------------------------------------------
  bool start() {
    // 1. Create UDP socket 创建 UDP Socket
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
      std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
      return false;
    }

    // 2. Set up server address 设置服务器地址
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort_);

    if (inet_pton(AF_INET, serverIp_.c_str(), &serverAddr.sin_addr) <= 0) {
      std::cerr << "Invalid address: " << serverIp_ << std::endl;
      close(sockfd_);
      sockfd_ = -1;
      return false;
    }

    // 3. Use connect() for UDP 对 UDP 使用 connect()
    //    This allows us to use send()/recv() instead of sendto()/recvfrom()
    //    这样可以使用 send()/recv() 而不是 sendto()/recvfrom()
    //    Also filters out packets from other sources
    //    也可以过滤来自其他来源的数据包
    if (connect(sockfd_, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
        0) {
      std::cerr << "connect() failed: " << strerror(errno) << std::endl;
      close(sockfd_);
      sockfd_ = -1;
      return false;
    }

    running_ = true;

    // 4. Send JOIN message 发送加入消息
    sendMessage(MessageType::JOIN, username_);

    printWelcome();
    lastHeartbeatSent_ = time(nullptr);

    return true;
  }

  // --------------------------------------------------------
  // Main event loop 主事件循环
  // --------------------------------------------------------
  void run() {
    char buffer[BUFFER_SIZE];

    while (running_) {
      fd_set readSet;
      FD_ZERO(&readSet);
      FD_SET(STDIN_FILENO, &readSet); // 监控键盘输入
      FD_SET(sockfd_, &readSet);      // 监控 socket

      int maxfd = (sockfd_ > STDIN_FILENO) ? sockfd_ : STDIN_FILENO;

      // Timeout for heartbeat 心跳超时
      struct timeval timeout;
      timeout.tv_sec = HEARTBEAT_INTERVAL;
      timeout.tv_usec = 0;

      int ready = select(maxfd + 1, &readSet, nullptr, nullptr, &timeout);

      if (ready < 0) {
        if (errno == EINTR) {
          continue; // Interrupted by signal 被信号中断
        }
        std::cerr << "select() error: " << strerror(errno) << std::endl;
        break;
      }

      // Send heartbeat periodically 定期发送心跳
      time_t now = time(nullptr);
      if (now - lastHeartbeatSent_ >= HEARTBEAT_INTERVAL) {
        sendMessage(MessageType::HEARTBEAT, "");
        lastHeartbeatSent_ = now;
      }

      if (ready == 0) {
        // Timeout, heartbeat already sent 超时，心跳已发送
        continue;
      }

      // Handle user input 处理用户输入
      if (FD_ISSET(STDIN_FILENO, &readSet)) {
        handleUserInput();
      }

      // Handle server message 处理服务器消息
      if (FD_ISSET(sockfd_, &readSet)) {
        ssize_t n = recv(sockfd_, buffer, sizeof(buffer), 0);

        if (n < 0) {
          if (errno == EINTR) {
            continue;
          }
          std::cerr << "recv() error: " << strerror(errno) << std::endl;
          continue;
        }

        if (n > 0) {
          handleServerMessage(buffer, n);
        }
      }
    }
  }

  // --------------------------------------------------------
  // Stop client 停止客户端
  // --------------------------------------------------------
  void stop() {
    if (running_ && sockfd_ >= 0) {
      // Send LEAVE message 发送离开消息
      sendMessage(MessageType::LEAVE, username_);
    }

    running_ = false;

    if (sockfd_ >= 0) {
      close(sockfd_);
      sockfd_ = -1;
    }

    std::cout << Color::YELLOW << "\nGoodbye! 再见！" << Color::RESET
              << std::endl;
  }

private:
  // --------------------------------------------------------
  // Print welcome message 打印欢迎信息
  // --------------------------------------------------------
  void printWelcome() {
    std::cout << Color::CYAN << "╔══════════════════════════════════════╗\n"
              << "║     Welcome to UDP Chat Room!        ║\n"
              << "║     欢迎来到 UDP 聊天室!              ║\n"
              << "║  Type message and press Enter        ║\n"
              << "║  Type /quit to exit 输入/quit退出    ║\n"
              << "║  Type /list to see users 输入/list   ║\n"
              << "╚══════════════════════════════════════╝" << Color::RESET
              << std::endl;
  }

  // --------------------------------------------------------
  // Handle user input 处理用户输入
  // --------------------------------------------------------
  void handleUserInput() {
    std::string line;
    if (!std::getline(std::cin, line)) {
      // EOF (Ctrl+D)
      running_ = false;
      return;
    }

    // Trim whitespace 去除首尾空白
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
      line.pop_back();
    }

    if (line.empty()) {
      return;
    }

    // Check for commands 检查命令
    if (line == "/quit" || line == "/exit") {
      running_ = false;
      return;
    }

    if (line == "/list" || line == "/users") {
      // Request user list 请求用户列表 (send empty chat, server will respond)
      sendMessage(MessageType::USER_LIST, "");
      return;
    }

    // Send chat message 发送聊天消息
    sendMessage(MessageType::CHAT, line);
  }

  // --------------------------------------------------------
  // Handle server message 处理服务器消息
  // --------------------------------------------------------
  void handleServerMessage(const char *data, size_t len) {
    MessageType type;
    std::string body;

    if (!parseMessage(data, len, type, body)) {
      return;
    }

    switch (type) {
    case MessageType::CHAT:
      std::cout << Color::BLUE << "[" << getCurrentTime() << "] " << body
                << Color::RESET << std::endl;
      break;

    case MessageType::SYSTEM:
      std::cout << Color::GREEN << "[" << getCurrentTime() << "] "
                << ">>> " << body << Color::RESET << std::endl;
      break;

    case MessageType::USER_LIST:
      std::cout << Color::CYAN << "[" << getCurrentTime() << "] " << body
                << Color::RESET << std::endl;
      break;

    case MessageType::HEARTBEAT_ACK:
      // Heartbeat acknowledged, connection is alive
      // 心跳确认，连接正常
      // (Silent, no output 静默，不输出)
      break;

    default:
      break;
    }
  }

  // --------------------------------------------------------
  // Send message to server 发送消息到服务器
  // --------------------------------------------------------
  bool sendMessage(MessageType type, const std::string &body) {
    if (sockfd_ < 0) {
      return false;
    }

    auto data = serializeMessage(type, body);

    // Since we used connect(), we can use send() instead of sendto()
    // 因为使用了 connect()，可以用 send() 而不是 sendto()
    ssize_t n = send(sockfd_, data.data(), data.size(), 0);

    if (n < 0) {
      std::cerr << "send() error: " << strerror(errno) << std::endl;
      return false;
    }

    return true;
  }
};

// ============================================================
// Global client instance for signal handler
// ============================================================

UDPChatClient *g_client = nullptr;

void signalHandler(int sig) {
  if (g_client) {
    g_client->stop();
  }
}

// ============================================================
// Main function 主函数
// ============================================================

int main(int argc, char *argv[]) {
  std::string serverIp = "127.0.0.1";
  int serverPort = DEFAULT_PORT;
  std::string username;

  // Parse command line arguments 解析命令行参数
  if (argc >= 2) {
    serverIp = argv[1];
  }
  if (argc >= 3) {
    serverPort = std::atoi(argv[2]);
  }
  if (argc >= 4) {
    username = argv[3];
  }

  // Get username if not provided 如果未提供用户名则获取
  if (username.empty()) {
    std::cout << "Enter your username 请输入用户名: ";
    std::getline(std::cin, username);
    if (username.empty()) {
      username = "User" + std::to_string(getpid());
    }
  }

  // Set up signal handler 设置信号处理
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  UDPChatClient client(serverIp, serverPort, username);
  g_client = &client;

  if (!client.start()) {
    return 1;
  }

  client.run();

  return 0;
}