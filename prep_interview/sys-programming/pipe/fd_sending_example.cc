#include <cstring>
#include <fcntl.h> // open
#include <iostream>
#include <sys/socket.h> // socketpair, sendmsg, recvmsg
#include <sys/wait.h>   // wait
#include <unistd.h>     // fork, read, write, close

/**
 * 发送文件描述符到另一个进程
 * Send file descriptor to another process
 *
 * @param sock   Unix domain socket (用于通信)
 * @param fd     要发送的文件描述符 (file descriptor to send)
 * @return       成功返回 0, 失败返回 -1
 */
int send_fd(int sock, int fd) {
  // ========== 1. 准备普通数据 ==========
  // Must send at least 1 byte of real data
  // 必须发送至少 1 字节的实际数据
  char buf[1] = {'F'}; // 'F' for "fd"

  struct iovec iov;
  iov.iov_base = buf;
  iov.iov_len = sizeof(buf);

  // ========== 2. 准备控制消息 ==========
  // Prepare control message for passing fd
  // CMSG_SPACE: 计算控制消息需要的空间
  char control[CMSG_SPACE(sizeof(int))];
  memset(control, 0, sizeof(control));

  // ========== 3. 填充 msghdr ==========
  struct msghdr msg = {0};
  msg.msg_iov = &iov; // 普通数据
  msg.msg_iovlen = 1;
  msg.msg_control = control; // 控制消息
  msg.msg_controllen = sizeof(control);

  // ========== 4. 填充 cmsghdr ==========
  // CMSG_FIRSTHDR: 获取第一个控制消息头
  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;          // Socket 层
  cmsg->cmsg_type = SCM_RIGHTS;           // 传递文件描述符
  cmsg->cmsg_len = CMSG_LEN(sizeof(int)); // 消息长度

  // CMSG_DATA: 获取控制消息数据区指针
  // 把 fd 放进去
  *reinterpret_cast<int *>(CMSG_DATA(cmsg)) = fd;

  // ========== 5. 发送 ==========
  ssize_t n = sendmsg(sock, &msg, 0);
  return (n >= 0) ? 0 : -1;
}

/**
 * 从另一个进程接收文件描述符
 * Receive file descriptor from another process
 *
 * @param sock   Unix domain socket (用于通信)
 * @return       收到的文件描述符, 失败返回 -1
 */
int recv_fd(int sock) {
  // ========== 1. 准备接收普通数据 ==========
  char buf[1];

  struct iovec iov;
  iov.iov_base = buf;
  iov.iov_len = sizeof(buf);

  // ========== 2. 准备接收控制消息 ==========
  char control[CMSG_SPACE(sizeof(int))];

  // ========== 3. 填充 msghdr ==========
  struct msghdr msg = {0};
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = control;
  msg.msg_controllen = sizeof(control);

  // ========== 4. 接收 ==========
  ssize_t n = recvmsg(sock, &msg, 0);
  if (n < 0) {
    return -1;
  }

  // ========== 5. 提取 fd ==========
  struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
  if (cmsg == nullptr || cmsg->cmsg_level != SOL_SOCKET ||
      cmsg->cmsg_type != SCM_RIGHTS) {
    return -1;
  }

  // 从控制消息中取出 fd
  return *reinterpret_cast<int *>(CMSG_DATA(cmsg));
}

int main() {
  // ========== 创建 socketpair ==========
  int sv[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
    perror("socketpair");
    return 1;
  }

  pid_t pid = fork();

  if (pid > 0) {
    // ========== Parent 父进程 ==========
    close(sv[1]);

    // 打开一个文件
    // Open a file
    int file_fd = open("test.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_fd == -1) {
      perror("open");
      return 1;
    }

    // 写入一些内容
    // Write some content
    const char *content = "Secret message from parent!\n";
    write(file_fd, content, strlen(content));

    // 重置文件位置到开头
    // Reset file position to beginning
    lseek(file_fd, 0, SEEK_SET);

    std::cout << "Parent: opened file, fd = " << file_fd << "\n";
    std::cout << "Parent: sending fd to child...\n";

    // 发送 fd 给子进程
    // Send fd to child
    if (send_fd(sv[0], file_fd) == -1) {
      perror("send_fd");
      return 1;
    }

    std::cout << "Parent: fd sent successfully\n";

    // 父进程可以关闭自己的 fd 了
    // Parent can close its own fd now
    close(file_fd);
    close(sv[0]);

    wait(nullptr);

  } else {
    // ========== Child 子进程 ==========
    close(sv[0]);

    std::cout << "Child: waiting for fd...\n";

    // 接收 fd
    // Receive fd
    int received_fd = recv_fd(sv[1]);
    if (received_fd == -1) {
      perror("recv_fd");
      return 1;
    }

    std::cout << "Child: received fd = " << received_fd << "\n";

    // 使用收到的 fd 读取文件内容
    // Use received fd to read file content
    char buf[256] = {0};
    ssize_t n = read(received_fd, buf, sizeof(buf) - 1);

    std::cout << "Child: read " << n << " bytes from file\n";
    std::cout << "Child: content = \"" << buf << "\"\n";

    close(received_fd);
    close(sv[1]);
  }

  return 0;
}