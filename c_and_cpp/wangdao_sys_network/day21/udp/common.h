// common.h
#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

// ============================================================
// Constants 常量定义
// ============================================================

constexpr int DEFAULT_PORT = 9999;
constexpr int BUFFER_SIZE = 1400; // Safe UDP size 安全的 UDP 大小
constexpr int MAX_USERNAME_LEN = 32;
constexpr int MAX_MESSAGE_LEN = 1024;

// Heartbeat Configuration 心跳配置
constexpr int HEARTBEAT_INTERVAL = 5; // 心跳间隔 (秒)
constexpr int HEARTBEAT_TIMEOUT = 30; // 心跳超时 (秒)
constexpr int CHECK_INTERVAL = 5;     // 服务端检查间隔 (秒)

// ============================================================
// Message Types 消息类型
// ============================================================

enum class MessageType : uint8_t {
  JOIN = 1,          // 加入聊天室 Join chat room
  LEAVE = 2,         // 离开聊天室 Leave chat room
  CHAT = 3,          // 聊天消息 Chat message
  HEARTBEAT = 4,     // 心跳请求 Heartbeat ping
  HEARTBEAT_ACK = 5, // 心跳响应 Heartbeat pong
  USER_LIST = 6,     // 用户列表 User list
  SYSTEM = 7,        // 系统消息 System message
};

// ============================================================
// Message Utility Functions 消息工具函数
// ============================================================

// Serialize message 序列化消息
// UDP 不需要长度前缀，一个数据报就是一条完整消息
inline std::vector<char> serializeMessage(MessageType type,
                                          const std::string &body) {
  std::vector<char> data(1 + body.size());
  data[0] = static_cast<char>(type);
  if (!body.empty()) {
    std::memcpy(data.data() + 1, body.data(), body.size());
  }
  return data;
}

// Parse message type 解析消息类型
inline bool parseMessage(const char *data, size_t len, MessageType &type,
                         std::string &body) {
  if (len < 1) {
    return false;
  }
  type = static_cast<MessageType>(static_cast<uint8_t>(data[0]));
  if (len > 1) {
    body.assign(data + 1, len - 1);
  } else {
    body.clear();
  }
  return true;
}

// Get current timestamp string 获取当前时间戳字符串
inline std::string getCurrentTime() {
  time_t now = time(nullptr);
  struct tm *tm_info = localtime(&now);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
  return std::string(buffer);
}

// Make address key from sockaddr_in 从地址生成键
inline std::string makeAddrKey(const struct sockaddr_in &addr) {
  return std::string(inet_ntoa(addr.sin_addr)) + ":" +
         std::to_string(ntohs(addr.sin_port));
}

// ============================================================
// Color Output 彩色输出 (ANSI Escape Codes)
// ============================================================

namespace Color {
const char *RESET = "\033[0m";
const char *RED = "\033[31m";
const char *GREEN = "\033[32m";
const char *YELLOW = "\033[33m";
const char *BLUE = "\033[34m";
const char *MAGENTA = "\033[35m";
const char *CYAN = "\033[36m";
const char *GRAY = "\033[90m";
} // namespace Color

#endif // COMMON_H