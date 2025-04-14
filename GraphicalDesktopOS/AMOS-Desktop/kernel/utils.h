#ifndef UTILS_H
#define UTILS_H

void sleep(int ms);
void* memset(void* s, int c, unsigned int n);
void* memcpy(void* dest, const void* src, unsigned int n);
void* memmove(void* dest, const void* src, unsigned int n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, unsigned int n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, unsigned int n);
char* strdup(const char* s);
unsigned int strlen(const char* s);

#endif /* UTILS_H */
