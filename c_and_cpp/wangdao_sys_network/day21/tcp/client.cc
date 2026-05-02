// client.cpp
#include "common.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cmath>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

// ============================================================
// Chat Client Class 聊天客户端类
// ============================================================

class ChatClient {
private:
  int sockfd_;                   // Socket fd
  std::string serverIp_;         // 服务器 IP
  int serverPort_;               // 服务器端口
  std::string username_;         // 用户名
  std::vector<char> recvBuffer_; // 接收缓冲区
  bool running_;                 // 运行标志
  bool connected_;               // 连接状态

  // Heartbeat 心跳相关
  time_t lastHeartbeatSent_; // 最后发送心跳时间
  time_t lastHeartbeatRecv_; // 最后收到心跳响应时间
  int missedHeartbeats_;     // 丢失的心跳次数

  // Reconnection 重连相关
  int reconnectAttempts_; // 重连尝试次数

public:
  ChatClient(const std::string &ip, int port, const std::string &username)
      : sockfd_(-1), serverIp_(ip), serverPort_(port), username_(username),
        running_(false), connected_(false), lastHeartbeatSent_(0),
        lastHeartbeatRecv_(0), missedHeartbeats_(0), reconnectAttempts_(0) {}

  ~ChatClient() { disconnect(); }

  // --------------------------------------------------------
  // Connect to server 连接到服务器
  // --------------------------------------------------------
  bool connect() {
    // Create socket 创建 Socket
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
      std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
      return false;
    }

    // Set up server address 设置服务器地址
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

    // Connect to server 连接服务器
    if (::connect(sockfd_, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
        0) {
      std::cerr << "Failed to connect: " << strerror(errno) << std::endl;
      close(sockfd_);
      sockfd_ = -1;
      return false;
    }

    connected_ = true;
    lastHeartbeatRecv_ = time(nullptr);
    missedHeartbeats_ = 0;

    std::cout << Color::GREEN << "Connected to server 已连接到服务器 "
              << serverIp_ << ":" << serverPort_ << Color::RESET << std::endl;

    // Send username 发送用户名
    sendMessage(MessageType::SET_NAME, username_);

    return true;
  }

  // --------------------------------------------------------
  // Disconnect from server 断开连接
  // --------------------------------------------------------
  void disconnect() {
    connected_ = false;
    if (sockfd_ >= 0) {
      close(sockfd_);
      sockfd_ = -1;
    }
  }

  // --------------------------------------------------------
  // Start client 启动客户端
  // --------------------------------------------------------
  bool start() {
    if (!connect()) {
      return false;
    }

    running_ = true;
    printWelcome();

    return true;
  }

  // --------------------------------------------------------
  // Main event loop 主事件循环
  // --------------------------------------------------------
  void run() {
    while (running_) {
      // Check if connected 检查是否连接
      if (!connected_) {
        // Attempt reconnection 尝试重连
        if (!attemptReconnect()) {
          std::cout << Color::RED
                    << "Failed to reconnect, giving up. 重连失败，放弃。"
                    << Color::RESET << std::endl;
          running_ = false;
          break;
        }
        continue;
      }

      fd_set readSet;
      FD_ZERO(&readSet);
      FD_SET(STDIN_FILENO, &readSet); // 监控标准输入 (键盘)
      FD_SET(sockfd_, &readSet);      // 监控 Socket

      int maxfd = std::max(STDIN_FILENO, sockfd_);

      // Timeout for heartbeat 心跳超时
      struct timeval timeout;
      timeout.tv_sec = HEARTBEAT_INTERVAL;
      timeout.tv_usec = 0;

      int ready = select(maxfd + 1, &readSet, nullptr, nullptr, &timeout);

      if (ready < 0) {
        if (errno == EINTR) {
          continue; // Interrupted by signal
        }
        std::cerr << "select() error: " << strerror(errno) << std::endl;
        break;
      }

      if (ready == 0) {
        // Timeout - send heartbeat 超时 - 发送心跳
        sendHeartbeat();
        checkHeartbeatTimeout();
        continue;
      }

      // Handle user input 处理用户输入
      if (FD_ISSET(STDIN_FILENO, &readSet)) {
        handleUserInput();
      }

      // Handle server data 处理服务器数据
      if (FD_ISSET(sockfd_, &readSet)) {
        handleServerData();
      }

      // Check heartbeat timeout 检查心跳超时
      checkHeartbeatTimeout();
    }
  }

  // --------------------------------------------------------
  // Stop client 停止客户端
  // --------------------------------------------------------
  void stop() {
    running_ = false;
    disconnect();
    std::cout << Color::YELLOW << "\nGoodbye! 再见！" << Color::RESET
              << std::endl;
  }

private:
  // --------------------------------------------------------
  // Print welcome message 打印欢迎信息
  // --------------------------------------------------------
  void printWelcome() {
    std::cout << Color::CYAN << "╔══════════════════════════════════════╗\n"
              << "║       Welcome to Chat Room!          ║\n"
              << "║         欢迎来到聊天室!               ║\n"
              << "║  Type message and press Enter        ║\n"
              << "║  Type /quit to exit 输入/quit退出    ║\n"
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

    // Check for quit command 检查退出命令
    if (line == "/quit" || line == "/exit") {
      running_ = false;
      return;
    }

    // Send message 发送消息
    sendMessage(MessageType::CHAT, line);
  }

  // --------------------------------------------------------
  // Handle server data 处理服务器数据
  // --------------------------------------------------------
  void handleServerData() {
    char buffer[BUFFER_SIZE];

    ssize_t n = recv(sockfd_, buffer, sizeof(buffer), 0);

    if (n <= 0) {
      if (n == 0) {
        std::cout << Color::YELLOW << "\nServer disconnected 服务器断开连接"
                  << Color::RESET << std::endl;
      } else {
        std::cerr << "recv() error: " << strerror(errno) << std::endl;
      }

      // Mark as disconnected, will trigger reconnect
      // 标记为断开，将触发重连
      connected_ = false;
      return;
    }

    // Append to buffer 追加到缓冲区
    recvBuffer_.insert(recvBuffer_.end(), buffer, buffer + n);

    // Process messages 处理消息
    processMessages();
  }

  // --------------------------------------------------------
  // Process received messages 处理接收到的消息
  // --------------------------------------------------------
  void processMessages() {
    while (true) {
      if (recvBuffer_.size() < HEADER_SIZE) {
        break;
      }

      MessageHeader header;
      if (!parseHeader(recvBuffer_, header)) {
        break;
      }

      size_t totalSize = HEADER_SIZE + header.length;
      if (recvBuffer_.size() < totalSize) {
        break;
      }

      std::string body(recvBuffer_.begin() + HEADER_SIZE,
                       recvBuffer_.begin() + totalSize);

      recvBuffer_.erase(recvBuffer_.begin(), recvBuffer_.begin() + totalSize);

      handleMessage(static_cast<MessageType>(header.type), body);
    }
  }

  // --------------------------------------------------------
  // Handle a single message 处理单条消息
  // --------------------------------------------------------
  void handleMessage(MessageType type, const std::string &body) {
    switch (type) {
    case MessageType::CHAT:
      std::cout << Color::BLUE << "[" << getCurrentTime() << "] " << body
                << Color::RESET << std::endl;
      break;

    case MessageType::JOIN:
      std::cout << Color::GREEN << "[" << getCurrentTime() << "] "
                << ">>> " << body << Color::RESET << std::endl;
      break;

    case MessageType::LEAVE:
      std::cout << Color::YELLOW << "[" << getCurrentTime() << "] "
                << "<<< " << body << Color::RESET << std::endl;
      break;

    case MessageType::SYSTEM:
      std::cout << Color::MAGENTA << "[System 系统] " << body << Color::RESET
                << std::endl;
      break;

    case MessageType::HEARTBEAT_ACK:
      // Received heartbeat response 收到心跳响应
      lastHeartbeatRecv_ = time(nullptr);
      missedHeartbeats_ = 0;
      break;

    default:
      break;
    }
  }

  // --------------------------------------------------------
  // Send message to server 发送消息到服务器
  // --------------------------------------------------------
  bool sendMessage(MessageType type, const std::string &body) {
    if (!connected_ || sockfd_ < 0) {
      return false;
    }

    auto data = serializeMessage(type, body);

    size_t sent = 0;
    while (sent < data.size()) {
      ssize_t n = send(sockfd_, data.data() + sent, data.size() - sent, 0);
      if (n <= 0) {
        if (n < 0 && errno == EINTR) {
          continue;
        }
        connected_ = false;
        return false;
      }
      sent += n;
    }

    return true;
  }

  // --------------------------------------------------------
  // Send heartbeat 发送心跳
  // --------------------------------------------------------
  void sendHeartbeat() {
    time_t now = time(nullptr);
    if (now - lastHeartbeatSent_ >= HEARTBEAT_INTERVAL) {
      sendMessage(MessageType::HEARTBEAT, "");
      lastHeartbeatSent_ = now;
    }
  }

  // --------------------------------------------------------
  // Check heartbeat timeout 检查心跳超时
  // --------------------------------------------------------
  void checkHeartbeatTimeout() {
    time_t now = time(nullptr);

    if (now - lastHeartbeatRecv_ > HEARTBEAT_TIMEOUT) {
      std::cout << Color::YELLOW
                << "\nHeartbeat timeout, connection may be lost."
                << "\n心跳超时，连接可能已断开。" << Color::RESET << std::endl;

      connected_ = false;
    }
  }

  // --------------------------------------------------------
  // Attempt reconnection 尝试重连
  // --------------------------------------------------------
  bool attemptReconnect() {
    // Check max attempts 检查最大重试次数
    if (reconnectAttempts_ >= RECONNECT_MAX_ATTEMPTS) {
      return false;
    }

    reconnectAttempts_++;

    // Calculate delay using Exponential Backoff
    // 使用指数退避计算延迟
    int delay =
        std::min(RECONNECT_BASE_DELAY *
                     static_cast<int>(std::pow(2, reconnectAttempts_ - 1)),
                 RECONNECT_MAX_DELAY);

    std::cout << Color::CYAN << "Reconnecting in " << delay << " seconds... "
              << "(Attempt " << reconnectAttempts_ << "/"
              << RECONNECT_MAX_ATTEMPTS << ")"
              << "\n"
              << delay << " 秒后重连... "
              << "(第 " << reconnectAttempts_ << "/" << RECONNECT_MAX_ATTEMPTS
              << " 次尝试)" << Color::RESET << std::endl;

    // Wait before reconnecting 重连前等待
    sleep(delay);

    // Close old socket if still open 关闭旧 Socket
    if (sockfd_ >= 0) {
      close(sockfd_);
      sockfd_ = -1;
    }

    // Clear receive buffer 清空接收缓冲区
    recvBuffer_.clear();

    // Attempt to connect 尝试连接
    if (connect()) {
      // Success! Reset counter 成功！重置计数器
      reconnectAttempts_ = 0;
      std::cout << Color::GREEN << "Reconnected successfully! 重连成功！"
                << Color::RESET << std::endl;
      return true;
    }

    return true; // Return true to continue trying 返回 true 继续尝试
  }
};

// ============================================================
// Global client instance for signal handler
// ============================================================

ChatClient *g_client = nullptr;

void signalHandler(int sig) {
  if (g_client) {
    g_client->stop();
  }
}

// ============================================================
// Main function 主函数
// ============================================================

int main(int argc, char *argv[]) {
  // Default values 默认值
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

  // Create and start client 创建并启动客户端
  ChatClient client(serverIp, serverPort, username);
  g_client = &client;

  if (!client.start()) {
    return 1;
  }

  client.run();

  return 0;
}
