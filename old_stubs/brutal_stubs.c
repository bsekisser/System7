// BRUTAL STUBS - Make everything compile
#include "SystemTypes.h"

// Add any function that's undefined
void* __stub_generic() { return 0; }

// Common stubs that are often missing
int printf(const char* fmt, ...) { return 0; }
int sprintf(char* buf, const char* fmt, ...) { return 0; }
int snprintf(char* buf, int size, const char* fmt, ...) { return 0; }
void* malloc(size_t size) { return 0; }
void free(void* ptr) { }
void* calloc(size_t n, size_t s) { return 0; }
void* realloc(void* p, size_t s) { return 0; }
int strcmp(const char* a, const char* b) { return 0; }
int strncmp(const char* a, const char* b, size_t n) { return 0; }
char* strcpy(char* d, const char* s) { return d; }
char* strncpy(char* d, const char* s, size_t n) { return d; }
size_t strlen(const char* s) { return 0; }
void* memcpy(void* d, const void* s, size_t n) { return d; }
void* memmove(void* d, const void* s, size_t n) { return d; }
void* memset(void* s, int c, size_t n) { return s; }
int memcmp(const void* a, const void* b, size_t n) { return 0; }
int abs(int n) { return n > 0 ? n : -n; }
long labs(long n) { return n > 0 ? n : -n; }
