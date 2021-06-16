#ifndef _HANDLE_ERROR_H
#define _HANDLE_ERROR_H

void DieWithUserMessage(const char *msg, const char *detail);

void DieWithSystemMessage(const char *msg);

void DieWithUserMessageClose(const char *msg, const char *detail, int fd);

void DieWithSystemMessageClose(const char *msg, int fd);

#endif
