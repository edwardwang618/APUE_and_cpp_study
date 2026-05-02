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

constexpr int DEFAULT_PORT = 8888;
constexpr int BUFFER_SIZE = 4096;
constexpr int MAX_USERNAME_LEN = 32;
constexpr int MAX_MESSAGE_LEN = 1024;

// Heartbeat Configuration 心跳配置
constexpr int HEARTBEAT_INTERVAL = 10; // 心跳间隔 (秒)
constexpr int HEARTBEAT_TIMEOUT = 30;  // 心跳超时 (秒)

// Reconnection Configuration 重连配置
constexpr int RECONNECT_BASE_DELAY = 1;    // 基础延迟 (秒)
constexpr int RECONNECT_MAX_DELAY = 60;    // 最大延迟 (秒)
constexpr int RECONNECT_MAX_ATTEMPTS = 10; // 最大重试次数

// ============================================================
// Message Types 消息类型
// ============================================================

enum class MessageType : uint8_t {
  CHAT = 1,          // 普通聊天消息 Chat message
  JOIN = 2,          // 用户加入 User joined
  LEAVE = 3,         // 用户离开 User left
  SYSTEM = 4,        // 系统消息 System message
  HEARTBEAT = 5,     // 心跳 Heartbeat ping
  HEARTBEAT_ACK = 6, // 心跳响应 Heartbeat pong
  SET_NAME = 7       // 设置用户名 Set username
};

// ============================================================
// Message Structure 消息结构
// ============================================================

// Message Header (5 bytes)
// 消息头 (5 字节)
#pragma pack(push, 1)
struct MessageHeader {
  uint32_t length; // Body length (Network byte order) 消息体长度
  uint8_t type;    // Message type 消息类型
};
#pragma pack(pop)

constexpr size_t HEADER_SIZE = sizeof(MessageHeader);

// ============================================================
// Message Utility Functions 消息工具函数
// ============================================================

// Serialize message 序列化消息
// 将消息类型和内容打包成可发送的字节流
inline std::vector<char> serializeMessage(MessageType type,
                                          const std::string &body) {
  std::vector<char> data(HEADER_SIZE + body.size());

  MessageHeader header;
  header.length = htonl(static_cast<uint32_t>(body.size()));
  header.type = static_cast<uint8_t>(type);

  // Copy header 复制消息头
  std::memcpy(data.data(), &header, HEADER_SIZE);

  // Copy body 复制消息体
  if (!body.empty()) {
    std::memcpy(data.data() + HEADER_SIZE, body.data(), body.size());
  }

  return data;
}

// Parse header from buffer 从缓冲区解析消息头
// Returns true if header is complete 如果头部完整则返回 true
inline bool parseHeader(const std::vector<char> &buffer,
                        MessageHeader &header) {
  if (buffer.size() < HEADER_SIZE) {
    return false;
  }

  std::memcpy(&header, buffer.data(), HEADER_SIZE);
  header.length = ntohl(header.length); // Network to host byte order

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
