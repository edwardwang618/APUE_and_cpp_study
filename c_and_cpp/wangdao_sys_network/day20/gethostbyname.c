#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <hostname>\n", argv[0]);
    return 1;
  }

  struct hostent *pHost = gethostbyname(argv[1]);

  if (pHost == NULL) {
    herror("gethostbyname");
    return 1;
  }

  printf("Official name: %s\n", pHost->h_name);

  // Print all aliases
  printf("Aliases: ");
  for (char **p = pHost->h_aliases; *p != NULL; p++) {
    printf("%s ", *p);
  }
  printf("\n");

  // Print all IP addresses
  printf("IP addresses:\n");
  for (char **p = pHost->h_addr_list; *p != NULL; p++) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, *p, ip, sizeof(ip));
    printf("  %s\n", ip);
  }

  return 0;
}