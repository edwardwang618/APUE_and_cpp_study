// server.cpp
#include "common.h"

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

// ============================================================
// Client Information Structure 客户端信息结构
// ============================================================

struct ClientInfo {
  std::string name;        // 用户名 Username
  struct sockaddr_in addr; // 客户端地址 Client address
  time_t lastSeen;         // 最后活跃时间 Last seen time

  ClientInfo() : lastSeen(time(nullptr)) {
    std::memset(&addr, 0, sizeof(addr));
  }
};

// ============================================================
// UDP Chat Server Class UDP 聊天服务器类
// ============================================================

class UDPChatServer {
private:
  int sockfd_;                                // UDP Socket (只有一个!)
  int port_;                                  // 监听端口
  bool running_;                              // 运行标志
  std::map<std::string, ClientInfo> clients_; // 客户端列表 (key: "ip:port")

public:
  UDPChatServer(int port = DEFAULT_PORT)
      : sockfd_(-1), port_(port), running_(false) {}

  ~UDPChatServer() { stop(); }

  // --------------------------------------------------------
  // Start server 启动服务器
  // --------------------------------------------------------
  bool start() {
    // 1. Create UDP socket 创建 UDP Socket
    //    SOCK_DGRAM = UDP (not SOCK_STREAM which is TCP)
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
      std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
      return false;
    }

    // 2. Set SO_REUSEADDR 设置地址重用
    int opt = 1;
    if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno)
                << std::endl;
      close(sockfd_);
      return false;
    }

    // 3. Bind to port 绑定端口
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡
    serverAddr.sin_port = htons(port_);

    if (bind(sockfd_, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
      std::cerr << "Failed to bind: " << strerror(errno) << std::endl;
      close(sockfd_);
      return false;
    }

    running_ = true;

    std::cout << Color::GREEN << "╔══════════════════════════════════════╗\n"
              << "║   UDP Chat Server Started            ║\n"
              << "║   UDP 聊天服务器已启动                ║\n"
              << "║   Port 端口: " << port_ << "                      ║\n"
              << "╚══════════════════════════════════════╝" << Color::RESET
              << std::endl;

    return true;
  }

  // --------------------------------------------------------
  // Main event loop 主事件循环
  // --------------------------------------------------------
  void run() {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t addrLen;

    while (running_) {
      // Use select with timeout for heartbeat checking
      // 使用 select 带超时，用于心跳检查
      fd_set readSet;
      FD_ZERO(&readSet);
      FD_SET(sockfd_, &readSet);

      struct timeval timeout;
      timeout.tv_sec = CHECK_INTERVAL;
      timeout.tv_usec = 0;

      int ready = select(sockfd_ + 1, &readSet, nullptr, nullptr, &timeout);

      if (ready < 0) {
        if (errno == EINTR) {
          continue; // Interrupted by signal 被信号中断
        }
        std::cerr << "select() error: " << strerror(errno) << std::endl;
        break;
      }

      // Check heartbeat timeouts 检查心跳超时
      checkHeartbeats();

      if (ready == 0) {
        // Timeout, no data - just continue (heartbeat already checked)
        // 超时，无数据 - 继续 (心跳已检查)
        continue;
      }

      // Receive data 接收数据
      addrLen = sizeof(clientAddr);
      ssize_t n = recvfrom(sockfd_, buffer, sizeof(buffer), 0,
                           (struct sockaddr *)&clientAddr, &addrLen);

      if (n < 0) {
        if (errno == EINTR) {
          continue;
        }
        std::cerr << "recvfrom() error: " << strerror(errno) << std::endl;
        continue;
      }

      if (n == 0) {
        continue; // Empty datagram 空数据报
      }

      // Process message 处理消息
      processMessage(buffer, n, clientAddr);
    }
  }

  // --------------------------------------------------------
  // Stop server 停止服务器
  // --------------------------------------------------------
  void stop() {
    running_ = false;

    // Send leave notification to all clients 通知所有客户端
    auto msg = serializeMessage(MessageType::SYSTEM,
                                "Server shutting down 服务器关闭");
    for (auto &[key, client] : clients_) {
      sendTo(client.addr, msg);
    }

    clients_.clear();

    if (sockfd_ >= 0) {
      close(sockfd_);
      sockfd_ = -1;
    }

    std::cout << Color::YELLOW << "\nServer stopped 服务器已停止"
              << Color::RESET << std::endl;
  }

private:
  // --------------------------------------------------------
  // Process received message 处理接收到的消息
  // --------------------------------------------------------
  void processMessage(const char *data, size_t len,
                      const struct sockaddr_in &clientAddr) {
    MessageType type;
    std::string body;

    if (!parseMessage(data, len, type, body)) {
      return; // Invalid message 无效消息
    }

    std::string key = makeAddrKey(clientAddr);

    // Update lastSeen for any message type
    // 任何消息都更新最后活跃时间
    if (clients_.find(key) != clients_.end()) {
      clients_[key].lastSeen = time(nullptr);
    }

    switch (type) {
    case MessageType::JOIN:
      handleJoin(key, body, clientAddr);
      break;

    case MessageType::LEAVE:
      handleLeave(key);
      break;

    case MessageType::CHAT:
      handleChat(key, body);
      break;

    case MessageType::HEARTBEAT:
      handleHeartbeat(key, clientAddr);
      break;

    default:
      // Unknown message type 未知消息类型
      break;
    }
  }

  // --------------------------------------------------------
  // Handle JOIN message 处理加入消息
  // --------------------------------------------------------
  void handleJoin(const std::string &key, const std::string &name,
                  const struct sockaddr_in &addr) {
    // Check if already exists 检查是否已存在
    if (clients_.find(key) != clients_.end()) {
      // Client rejoining (maybe port changed) 客户端重新加入
      clients_[key].lastSeen = time(nullptr);
      return;
    }

    // Add new client 添加新客户端
    ClientInfo info;
    info.name = name.empty() ? "Anonymous" : name.substr(0, MAX_USERNAME_LEN);
    info.addr = addr;
    info.lastSeen = time(nullptr);
    clients_[key] = info;

    std::cout << Color::GREEN << "[" << getCurrentTime() << "] "
              << "User joined 用户加入: " << info.name << " (" << key << ")"
              << Color::RESET << std::endl;

    // Broadcast join notification 广播加入通知
    std::string joinMsg = info.name + " has joined the chat 加入了聊天室";
    broadcast(MessageType::SYSTEM, joinMsg, key);

    // Send user list to new client 发送用户列表给新客户端
    sendUserList(key);
  }

  // --------------------------------------------------------
  // Handle LEAVE message 处理离开消息
  // --------------------------------------------------------
  void handleLeave(const std::string &key) {
    if (clients_.find(key) == clients_.end()) {
      return;
    }

    std::string name = clients_[key].name;
    clients_.erase(key);

    std::cout << Color::YELLOW << "[" << getCurrentTime() << "] "
              << "User left 用户离开: " << name << Color::RESET << std::endl;

    // Broadcast leave notification 广播离开通知
    std::string leaveMsg = name + " has left the chat 离开了聊天室";
    broadcast(MessageType::SYSTEM, leaveMsg, "");
  }

  // --------------------------------------------------------
  // Handle CHAT message 处理聊天消息
  // --------------------------------------------------------
  void handleChat(const std::string &key, const std::string &message) {
    if (clients_.find(key) == clients_.end()) {
      return; // Unknown client 未知客户端
    }

    std::string name = clients_[key].name;

    std::cout << Color::BLUE << "[" << getCurrentTime() << "] " << name << ": "
              << message << Color::RESET << std::endl;

    // Format: "username: message"
    std::string chatMsg = name + ": " + message;
    broadcast(MessageType::CHAT, chatMsg, key); // Exclude sender 排除发送者
  }

  // --------------------------------------------------------
  // Handle HEARTBEAT message 处理心跳消息
  // --------------------------------------------------------
  void handleHeartbeat(const std::string &key, const struct sockaddr_in &addr) {
    // Update lastSeen (already done in processMessage)
    // 更新活跃时间 (已在 processMessage 中完成)

    // If client not in list, it might be a rejoin
    // 如果客户端不在列表中，可能是重新加入
    if (clients_.find(key) == clients_.end()) {
      // Client needs to JOIN first, ignore heartbeat
      // 客户端需要先 JOIN，忽略心跳
      return;
    }

    // Send heartbeat ACK 发送心跳响应
    auto ack = serializeMessage(MessageType::HEARTBEAT_ACK, "");
    sendTo(addr, ack);
  }

  // --------------------------------------------------------
  // Check heartbeat timeouts 检查心跳超时
  // --------------------------------------------------------
  void checkHeartbeats() {
    time_t now = time(nullptr);
    std::vector<std::string> deadClients;

    for (auto &[key, client] : clients_) {
      if (now - client.lastSeen > HEARTBEAT_TIMEOUT) {
        deadClients.push_back(key);
      }
    }

    // Remove dead clients 移除死亡客户端
    for (const auto &key : deadClients) {
      std::string name = clients_[key].name;
      clients_.erase(key);

      std::cout << Color::YELLOW << "[" << getCurrentTime() << "] "
                << "User timed out 用户超时: " << name << " (" << key << ")"
                << Color::RESET << std::endl;

      // Broadcast timeout notification 广播超时通知
      std::string msg = name + " timed out 超时离开";
      broadcast(MessageType::SYSTEM, msg, "");
    }
  }

  // --------------------------------------------------------
  // Send message to specific client 发送消息给特定客户端
  // --------------------------------------------------------
  void sendTo(const struct sockaddr_in &addr, const std::vector<char> &data) {
    sendto(sockfd_, data.data(), data.size(), 0, (struct sockaddr *)&addr,
           sizeof(addr));
  }

  // --------------------------------------------------------
  // Broadcast message to all clients 广播消息给所有客户端
  // --------------------------------------------------------
  void broadcast(MessageType type, const std::string &body,
                 const std::string &excludeKey) {
    auto data = serializeMessage(type, body);

    for (auto &[key, client] : clients_) {
      if (key != excludeKey) {
        sendTo(client.addr, data);
      }
    }
  }

  // --------------------------------------------------------
  // Send user list to client 发送用户列表给客户端
  // --------------------------------------------------------
  void sendUserList(const std::string &targetKey) {
    if (clients_.find(targetKey) == clients_.end()) {
      return;
    }

    // Build user list 构建用户列表
    std::string userList = "Online users 在线用户: ";
    bool first = true;
    for (auto &[key, client] : clients_) {
      if (!first) {
        userList += ", ";
      }
      userList += client.name;
      first = false;
    }

    auto data = serializeMessage(MessageType::USER_LIST, userList);
    sendTo(clients_[targetKey].addr, data);
  }
};

// ============================================================
// Global server instance for signal handler
// ============================================================

UDPChatServer *g_server = nullptr;

void signalHandler(int sig) {
  std::cout << "\nReceived signal " << sig << std::endl;
  if (g_server) {
    g_server->stop();
  }
}

// ============================================================
// Main function 主函数
// ============================================================

int main(int argc, char *argv[]) {
  int port = DEFAULT_PORT;
  if (argc > 1) {
    port = std::atoi(argv[1]);
  }

  // Set up signal handler 设置信号处理
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  UDPChatServer server(port);
  g_server = &server;

  if (!server.start()) {
    return 1;
  }

  server.run();

  return 0;
}