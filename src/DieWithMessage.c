#include "../include/HandleError.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void DieWithUserMessage(const char *msg, const char *detail) {
  fputs(msg, stderr);
  fputs(": ", stderr);
  fputs(detail, stderr);
  fputc('\n', stderr);
  exit(1);
}

void DieWithSystemMessage(const char *msg) {
  perror(msg);
  exit(1);
}

void DieWithUserMessageClose(const char *msg, const char *detail, int fd) {
  fputs(msg, stderr);
  fputs(": ", stderr);
  fputs(detail, stderr);
  fputc('\n', stderr);
  close(fd);
  exit(1);
}

void DieWithSystemMessageClose(const char *msg, int fd) {
  perror(msg);
  close(fd);
  exit(1);
}
