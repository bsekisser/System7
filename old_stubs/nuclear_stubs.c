// NUCLEAR STUBS - Every possible undefined symbol

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned int size_t;

// Memory functions
void* memcpy(void* d, const void* s, size_t n) {
    uint8_t* dd = d;
    const uint8_t* ss = s;
    while (n--) *dd++ = *ss++;
    return d;
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p = s;
    while (n--) *p++ = c;
    return s;
}

void* memmove(void* d, const void* s, size_t n) {
    return memcpy(d, s, n);
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = s1;
    const uint8_t* p2 = s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

// String functions
size_t strlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

char* strcpy(char* d, const char* s) {
    char* r = d;
    while ((*d++ = *s++));
    return r;
}

char* strncpy(char* d, const char* s, size_t n) {
    char* r = d;
    while (n-- && (*d++ = *s++));
    while (n--) *d++ = 0;
    return r;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2) { s1++; s2++; }
    return *(uint8_t*)s1 - *(uint8_t*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n-- && *s1 && *s1 == *s2) { s1++; s2++; }
    return n == (size_t)-1 ? 0 : *(uint8_t*)s1 - *(uint8_t*)s2;
}

char* strcat(char* d, const char* s) {
    char* r = d;
    while (*d) d++;
    while ((*d++ = *s++));
    return r;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == c) return (char*)s;
        s++;
    }
    return 0;
}

char* strrchr(const char* s, int c) {
    const char* last = 0;
    while (*s) {
        if (*s == c) last = s;
        s++;
    }
    return (char*)last;
}

char* strstr(const char* h, const char* n) {
    if (!*n) return (char*)h;
    while (*h) {
        const char* hh = h;
        const char* nn = n;
        while (*hh && *nn && *hh == *nn) { hh++; nn++; }
        if (!*nn) return (char*)h;
        h++;
    }
    return 0;
}

// Math/conversion
int atoi(const char* s) {
    int r = 0, sign = 1;
    if (*s == '-') { sign = -1; s++; }
    while (*s >= '0' && *s <= '9') {
        r = r * 10 + (*s - '0');
        s++;
    }
    return r * sign;
}

long atol(const char* s) { return (long)atoi(s); }

// Character functions
int isdigit(int c) { return c >= '0' && c <= '9'; }
int isalpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
int isalnum(int c) { return isdigit(c) || isalpha(c); }
int isspace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
int toupper(int c) { return (c >= 'a' && c <= 'z') ? c - 32 : c; }
int tolower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

// Memory allocation (all return NULL)
void* malloc(size_t s) { return 0; }
void free(void* p) { }
void* calloc(size_t n, size_t s) { return 0; }
void* realloc(void* p, size_t s) { return 0; }

// Process control
void abort(void) { while(1); }
void exit(int s) { while(1); }

// I/O stubs
int printf(const char* f, ...) { return 0; }
int sprintf(char* s, const char* f, ...) { return 0; }
int snprintf(char* s, size_t n, const char* f, ...) { return 0; }
int puts(const char* s) { return 0; }
int putchar(int c) { return c; }
int getchar(void) { return -1; }

// File I/O stubs
typedef struct FILE FILE;
FILE* stdin = 0;
FILE* stdout = 0;
FILE* stderr = 0;
FILE* fopen(const char* f, const char* m) { return 0; }
int fclose(FILE* f) { return 0; }
size_t fread(void* p, size_t s, size_t n, FILE* f) { return 0; }
size_t fwrite(const void* p, size_t s, size_t n, FILE* f) { return 0; }
int fgetc(FILE* f) { return -1; }
int fputc(int c, FILE* f) { return c; }
char* fgets(char* s, int n, FILE* f) { return 0; }
int fputs(const char* s, FILE* f) { return 0; }
int fprintf(FILE* f, const char* fmt, ...) { return 0; }
int fscanf(FILE* f, const char* fmt, ...) { return 0; }
void perror(const char* s) { }

// CATCH-ALL for any undefined symbol
void __undefined_symbol(void) {}

// Common Mac toolbox stubs (if needed)
void InitGraf(void* p) {}
void InitFonts(void) {}
void InitWindows(void) {}
void InitMenus(void) {}
void TEInit(void) {}
void InitDialogs(void* p) {}
void InitCursor(void) {}

// Add more as needed...

// Create aliases for common undefined symbols
void _start(void) __attribute__((alias("__undefined_symbol")));
void _init(void) __attribute__((alias("__undefined_symbol")));
void _fini(void) __attribute__((alias("__undefined_symbol")));

