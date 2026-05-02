// server.cpp
#include "common.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <set>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

// ============================================================
// Client Information Structure 客户端信息结构
// ============================================================

struct ClientInfo {
  std::string name;             // 用户名 Username
  std::string address;          // IP:Port 地址
  std::vector<char> recvBuffer; // 接收缓冲区 Receive buffer
  time_t lastHeartbeat;         // 最后心跳时间 Last heartbeat time

  ClientInfo() : name("Anonymous"), lastHeartbeat(time(nullptr)) {}
};

// ============================================================
// Chat Server Class 聊天服务器类
// ============================================================

class ChatServer {
private:
  int listenfd_;                      // Listening socket 监听 Socket
  int maxfd_;                         // 最大 fd 值
  fd_set masterSet_;                  // Master fd set 主 fd 集合
  std::map<int, ClientInfo> clients_; // Client map (fd → info)
  bool running_;                      // 运行标志
  int port_;                          // 监听端口

public:
  ChatServer(int port = DEFAULT_PORT)
      : listenfd_(-1), maxfd_(0), running_(false), port_(port) {
    FD_ZERO(&masterSet_);
  }

  ~ChatServer() { stop(); }

  // --------------------------------------------------------
  // Initialize and start server 初始化并启动服务器
  // --------------------------------------------------------
  bool start() {
    // 1. Create socket 创建 Socket
    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd_ < 0) {
      std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
      return false;
    }

    // 2. Set SO_REUSEADDR 设置地址重用
    //    允许服务器重启后立即绑定相同端口
    int opt = 1;
    if (setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
        0) {
      std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno)
                << std::endl;
      close(listenfd_);
      return false;
    }

    // 3. Bind address 绑定地址
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡
    serverAddr.sin_port = htons(port_);

    if (bind(listenfd_, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
        0) {
      std::cerr << "Failed to bind: " << strerror(errno) << std::endl;
      close(listenfd_);
      return false;
    }

    // 4. Start listening 开始监听
    if (listen(listenfd_, SOMAXCONN) < 0) {
      std::cerr << "Failed to listen: " << strerror(errno) << std::endl;
      close(listenfd_);
      return false;
    }

    // 5. Initialize select set 初始化 select 集合
    FD_SET(listenfd_, &masterSet_);
    maxfd_ = listenfd_;
    running_ = true;

    std::cout << Color::GREEN << "╔══════════════════════════════════════╗\n"
              << "║     Chat Server Started 服务器已启动  ║\n"
              << "║     Port 端口: " << port_ << "                    ║\n"
              << "╚══════════════════════════════════════╝" << Color::RESET
              << std::endl;

    return true;
  }

  // --------------------------------------------------------
  // Main event loop 主事件循环
  // --------------------------------------------------------
  void run() {
    while (running_) {
      // Copy master set for select (必须复制，因为 select 会修改)
      fd_set readSet = masterSet_;

      // Set timeout for heartbeat checking 设置超时用于心跳检查
      struct timeval timeout;
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;

      // Wait for events 等待事件
      int ready = select(maxfd_ + 1, &readSet, nullptr, nullptr, &timeout);

      if (ready < 0) {
        if (errno == EINTR) {
          // Interrupted by signal 被信号中断
          continue;
        }
        std::cerr << "select() error: " << strerror(errno) << std::endl;
        break;
      }

      if (ready == 0) {
        // Timeout - check heartbeats 超时 - 检查心跳
        checkHeartbeats();
        continue;
      }

      // Check all fds 检查所有 fd
      for (int fd = 0; fd <= maxfd_ && ready > 0; fd++) {
        if (!FD_ISSET(fd, &readSet)) {
          continue;
        }
        ready--;

        if (fd == listenfd_) {
          // New connection 新连接
          handleNewConnection();
        } else {
          // Client data 客户端数据
          handleClientData(fd);
        }
      }
    }
  }

  // --------------------------------------------------------
  // Stop server 停止服务器
  // --------------------------------------------------------
  void stop() {
    running_ = false;

    // Close all client connections 关闭所有客户端连接
    for (auto &[fd, client] : clients_) {
      close(fd);
    }
    clients_.clear();

    // Close listening socket 关闭监听 Socket
    if (listenfd_ >= 0) {
      close(listenfd_);
      listenfd_ = -1;
    }

    std::cout << Color::YELLOW << "\nServer stopped 服务器已停止"
              << Color::RESET << std::endl;
  }

private:
  // --------------------------------------------------------
  // Handle new connection 处理新连接
  // --------------------------------------------------------
  void handleNewConnection() {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // Accept new connection 接受新连接
    int clientfd = accept(listenfd_, (struct sockaddr *)&clientAddr, &addrLen);
    if (clientfd < 0) {
      std::cerr << "accept() error: " << strerror(errno) << std::endl;
      return;
    }

    // Check FD_SETSIZE limit 检查 fd 数量限制
    if (clientfd >= FD_SETSIZE) {
      std::cerr << "Too many clients (fd >= FD_SETSIZE)" << std::endl;
      close(clientfd);
      return;
    }

    // Add to select set 加入 select 监控集合
    FD_SET(clientfd, &masterSet_);
    maxfd_ = std::max(maxfd_, clientfd);

    // Create client info 创建客户端信息
    ClientInfo info;
    info.address = std::string(inet_ntoa(clientAddr.sin_addr)) + ":" +
                   std::to_string(ntohs(clientAddr.sin_port));
    info.lastHeartbeat = time(nullptr);
    clients_[clientfd] = info;

    std::cout << Color::CYAN << "[" << getCurrentTime() << "] "
              << "New connection from " << info.address << " (fd=" << clientfd
              << ")" << Color::RESET << std::endl;
  }

  // --------------------------------------------------------
  // Handle client data 处理客户端数据
  // --------------------------------------------------------
  void handleClientData(int clientfd) {
    char buffer[BUFFER_SIZE];

    // Receive data 接收数据
    ssize_t n = recv(clientfd, buffer, sizeof(buffer), 0);

    if (n <= 0) {
      // n == 0: Client disconnected 客户端断开
      // n < 0: Error 错误
      if (n == 0) {
        std::cout << Color::YELLOW << "[" << getCurrentTime() << "] "
                  << "Client disconnected: " << clients_[clientfd].name
                  << Color::RESET << std::endl;
      } else {
        std::cerr << "recv() error for fd " << clientfd << ": "
                  << strerror(errno) << std::endl;
      }
      removeClient(clientfd);
      return;
    }

    // Append to receive buffer 追加到接收缓冲区
    auto &client = clients_[clientfd];
    client.recvBuffer.insert(client.recvBuffer.end(), buffer, buffer + n);
    client.lastHeartbeat = time(nullptr); // 更新心跳时间

    // Process complete messages 处理完整消息
    processMessages(clientfd);
  }

  // --------------------------------------------------------
  // Process received messages 处理接收到的消息
  // --------------------------------------------------------
  void processMessages(int clientfd) {
    auto &client = clients_[clientfd];
    auto &buf = client.recvBuffer;

    while (true) {
      // Check if we have complete header 检查是否有完整的消息头
      if (buf.size() < HEADER_SIZE) {
        break;
      }

      // Parse header 解析消息头
      MessageHeader header;
      if (!parseHeader(buf, header)) {
        break;
      }

      // Check if we have complete message 检查是否有完整消息
      size_t totalSize = HEADER_SIZE + header.length;
      if (buf.size() < totalSize) {
        break; // 消息不完整，等待更多数据
      }

      // Extract message body 提取消息体
      std::string body(buf.begin() + HEADER_SIZE, buf.begin() + totalSize);

      // Remove processed data from buffer 从缓冲区移除已处理数据
      buf.erase(buf.begin(), buf.begin() + totalSize);

      // Handle message 处理消息
      handleMessage(clientfd, static_cast<MessageType>(header.type), body);
    }
  }

  // --------------------------------------------------------
  // Handle a single message 处理单条消息
  // --------------------------------------------------------
  void handleMessage(int clientfd, MessageType type, const std::string &body) {
    auto &client = clients_[clientfd];

    switch (type) {
    case MessageType::SET_NAME: {
      // Client setting username 客户端设置用户名
      std::string oldName = client.name;
      client.name = body.substr(0, MAX_USERNAME_LEN);

      std::cout << Color::GREEN << "[" << getCurrentTime() << "] "
                << "User joined: " << client.name << " (" << client.address
                << ")" << Color::RESET << std::endl;

      // Broadcast join message 广播加入消息
      std::string joinMsg = client.name + " has joined the chat 加入了聊天室";
      broadcast(MessageType::JOIN, joinMsg, -1); // 发给所有人
      break;
    }

    case MessageType::CHAT: {
      // Chat message 聊天消息
      std::cout << Color::BLUE << "[" << getCurrentTime() << "] " << client.name
                << ": " << body << Color::RESET << std::endl;

      // Format: "username: message"
      std::string chatMsg = client.name + ": " + body;
      broadcast(MessageType::CHAT, chatMsg, clientfd); // 不发给自己
      break;
    }

    case MessageType::HEARTBEAT: {
      // Respond with heartbeat ACK 响应心跳
      sendToClient(clientfd, MessageType::HEARTBEAT_ACK, "");
      break;
    }

    default:
      // Unknown message type 未知消息类型
      std::cerr << "Unknown message type: " << static_cast<int>(type)
                << std::endl;
      break;
    }
  }

  // --------------------------------------------------------
  // Send message to a specific client 发送消息给特定客户端
  // --------------------------------------------------------
  void sendToClient(int clientfd, MessageType type, const std::string &body) {
    auto data = serializeMessage(type, body);

    // Send all data 发送所有数据
    size_t sent = 0;
    while (sent < data.size()) {
      ssize_t n = send(clientfd, data.data() + sent, data.size() - sent, 0);
      if (n <= 0) {
        // Send failed 发送失败
        if (n < 0 && errno == EINTR) {
          continue; // Interrupted, retry
        }
        // Client disconnected 客户端断开
        removeClient(clientfd);
        return;
      }
      sent += n;
    }
  }

  // --------------------------------------------------------
  // Broadcast message to all clients 广播消息给所有客户端
  // --------------------------------------------------------
  void broadcast(MessageType type, const std::string &body, int excludefd) {
    for (auto &[fd, client] : clients_) {
      if (fd != excludefd) {
        sendToClient(fd, type, body);
      }
    }
  }

  // --------------------------------------------------------
  // Remove client 移除客户端
  // --------------------------------------------------------
  void removeClient(int clientfd) {
    if (clients_.find(clientfd) == clients_.end()) {
      return;
    }

    std::string name = clients_[clientfd].name;

    // Remove from select set 从 select 集合移除
    FD_CLR(clientfd, &masterSet_);

    // Close socket 关闭 Socket
    close(clientfd);

    // Remove from clients map 从客户端列表移除
    clients_.erase(clientfd);

    // Update maxfd 更新最大 fd
    while (maxfd_ > 0 && !FD_ISSET(maxfd_, &masterSet_)) {
      maxfd_--;
    }

    // Broadcast leave message 广播离开消息
    if (name != "Anonymous") {
      std::string leaveMsg = name + " has left the chat 离开了聊天室";
      broadcast(MessageType::LEAVE, leaveMsg, -1);
    }
  }

  // --------------------------------------------------------
  // Check heartbeats 检查心跳
  // --------------------------------------------------------
  void checkHeartbeats() {
    time_t now = time(nullptr);
    std::vector<int> timeoutClients;

    for (auto &[fd, client] : clients_) {
      if (now - client.lastHeartbeat > HEARTBEAT_TIMEOUT) {
        std::cout << Color::YELLOW << "[" << getCurrentTime() << "] "
                  << "Heartbeat timeout: " << client.name << Color::RESET
                  << std::endl;
        timeoutClients.push_back(fd);
      }
    }

    // Remove timed out clients 移除超时客户端
    for (int fd : timeoutClients) {
      removeClient(fd);
    }
  }
};

// ============================================================
// Global server instance for signal handler
// ============================================================

ChatServer *g_server = nullptr;

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

  ChatServer server(port);
  g_server = &server;

  if (!server.start()) {
    return 1;
  }

  server.run();

  return 0;
}
