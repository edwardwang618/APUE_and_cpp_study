#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // Check command line arguments
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <source> <dest>\n", argv[0]);
    exit(1);
  }

  // Open source file for reading
  FILE *src = fopen(argv[1], "r");
  if (src == NULL) {
    fprintf(stderr, "Error: open %s failed.\n", argv[1]);
    exit(1);
  }

  // Open destination file for writing
  FILE *dest = fopen(argv[2], "w");
  if (dest == NULL) {
    fprintf(stderr, "Error: open %s failed.\n", argv[2]);
    fclose(src);
    exit(1);
  }

  // Read and write data
  int ch;
  while ((ch = fgetc(src)) != EOF) {
    fputc(ch, dest);
  }

  // Close files
  fclose(src);
  fclose(dest);

  return 0;
}