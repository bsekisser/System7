#include "SystemTypes.h"

// Stub ALL undefined functions
void stub_undefined() { }
int stub_int() { return 0; }
void* stub_ptr() { return 0; }

// Common standard library functions
int printf(const char* fmt, ...) { return 0; }
int sprintf(char* buf, const char* fmt, ...) { return 0; }
int snprintf(char* buf, int size, const char* fmt, ...) { return 0; }
int sscanf(const char* str, const char* fmt, ...) { return 0; }
int vsprintf(char* buf, const char* fmt, void* args) { return 0; }
int vsnprintf(char* buf, int size, const char* fmt, void* args) { return 0; }

void* malloc(size_t size) { return 0; }
void free(void* ptr) { }
void* calloc(size_t n, size_t s) { return 0; }
void* realloc(void* p, size_t s) { return 0; }

int strcmp(const char* a, const char* b) { return 0; }
int strncmp(const char* a, const char* b, size_t n) { return 0; }
char* strcpy(char* d, const char* s) { return d; }
char* strncpy(char* d, const char* s, size_t n) { return d; }
char* strcat(char* d, const char* s) { return d; }
char* strncat(char* d, const char* s, size_t n) { return d; }
size_t strlen(const char* s) { return 0; }
char* strchr(const char* s, int c) { return (char*)s; }
char* strrchr(const char* s, int c) { return (char*)s; }
char* strstr(const char* h, const char* n) { return (char*)h; }

void* memcpy(void* d, const void* s, size_t n) { return d; }
void* memmove(void* d, const void* s, size_t n) { return d; }
void* memset(void* s, int c, size_t n) { return s; }
int memcmp(const void* a, const void* b, size_t n) { return 0; }
void* memchr(const void* s, int c, size_t n) { return (void*)s; }

int abs(int n) { return n > 0 ? n : -n; }
long labs(long n) { return n > 0 ? n : -n; }
double fabs(double x) { return x > 0 ? x : -x; }

int toupper(int c) { return c; }
int tolower(int c) { return c; }
int isalpha(int c) { return 0; }
int isdigit(int c) { return 0; }
int isspace(int c) { return 0; }
int isprint(int c) { return 0; }

int atoi(const char* str) { return 0; }
long atol(const char* str) { return 0; }
double atof(const char* str) { return 0.0; }

void exit(int status) { while(1); }
void abort() { while(1); }

// File I/O stubs
void* fopen(const char* path, const char* mode) { return 0; }
int fclose(void* stream) { return 0; }
int fread(void* ptr, int size, int n, void* stream) { return 0; }
int fwrite(const void* ptr, int size, int n, void* stream) { return 0; }
int fprintf(void* stream, const char* fmt, ...) { return 0; }
int fgetc(void* stream) { return -1; }
int fputc(int c, void* stream) { return c; }
char* fgets(char* s, int n, void* stream) { return 0; }
int fputs(const char* s, void* stream) { return 0; }
int fseek(void* stream, long offset, int whence) { return 0; }
long ftell(void* stream) { return 0; }
void rewind(void* stream) { }

// Time functions
unsigned long time(void* t) { return 0; }
int gettimeofday(void* tv, void* tz) { return 0; }

// Math functions
double sin(double x) { return 0; }
double cos(double x) { return 0; }
double tan(double x) { return 0; }
double sqrt(double x) { return 0; }
double pow(double x, double y) { return 0; }
double exp(double x) { return 0; }
double log(double x) { return 0; }
double ceil(double x) { return 0; }
double floor(double x) { return 0; }

// Random
int rand() { return 0; }
void srand(unsigned int seed) { }

// All other missing functions get generic stubs
void __stub_void() { }
int __stub_int_return() { return 0; }
void* __stub_ptr_return() { return 0; }


// HAL stubs
void HAL_Init() { }
void HAL_Shutdown() { }
void* HAL_GetContext() { return 0; }
int HAL_SetMode(int mode) { return 0; }
void HAL_ClearScreen() { }
void HAL_DrawPixel(int x, int y, int color) { }
void HAL_DrawLine(int x1, int y1, int x2, int y2, int color) { }
void HAL_DrawRect(int x, int y, int w, int h, int color) { }
void HAL_FillRect(int x, int y, int w, int h, int color) { }
void HAL_DrawText(const char* text, int x, int y, int color) { }

// Manager HAL stubs
void* MemoryMgr_HAL_Init(void* config) { return 0; }
void MemoryMgr_HAL_Shutdown(void* ctx) { }
void* ProcessMgr_HAL_Init(void* config) { return 0; }
void ProcessMgr_HAL_Shutdown(void* ctx) { }
void* QuickDraw_HAL_Init(void* config) { return 0; }
void QuickDraw_HAL_Shutdown(void* ctx) { }
void* WindowMgr_HAL_Init(void* config) { return 0; }
void WindowMgr_HAL_Shutdown(void* ctx) { }
void* EventMgr_HAL_Init(void* config) { return 0; }
void EventMgr_HAL_Shutdown(void* ctx) { }
void* MenuMgr_HAL_Init(void* config) { return 0; }
void MenuMgr_HAL_Shutdown(void* ctx) { }
void* ControlMgr_HAL_Init(void* config) { return 0; }
void ControlMgr_HAL_Shutdown(void* ctx) { }
void* DialogMgr_HAL_Init(void* config) { return 0; }
void DialogMgr_HAL_Shutdown(void* ctx) { }
void* TextEdit_HAL_Init(void* config) { return 0; }
void TextEdit_HAL_Shutdown(void* ctx) { }
void* ResourceMgr_HAL_Init(void* config) { return 0; }
void ResourceMgr_HAL_Shutdown(void* ctx) { }
void* FileMgr_HAL_Init(void* config) { return 0; }
void FileMgr_HAL_Shutdown(void* ctx) { }
void* SoundMgr_HAL_Init(void* config) { return 0; }
void SoundMgr_HAL_Shutdown(void* ctx) { }
void* StandardFile_HAL_Init(void* config) { return 0; }
void StandardFile_HAL_Shutdown(void* ctx) { }
void* ListMgr_HAL_Init(void* config) { return 0; }
void ListMgr_HAL_Shutdown(void* ctx) { }
void* ScrapMgr_HAL_Init(void* config) { return 0; }
void ScrapMgr_HAL_Shutdown(void* ctx) { }
void* PrintMgr_HAL_Init(void* config) { return 0; }
void PrintMgr_HAL_Shutdown(void* ctx) { }
void* HelpMgr_HAL_Init(void* config) { return 0; }
void HelpMgr_HAL_Shutdown(void* ctx) { }
void* ColorMgr_HAL_Init(void* config) { return 0; }
void ColorMgr_HAL_Shutdown(void* ctx) { }
void* ComponentMgr_HAL_Init(void* config) { return 0; }
void ComponentMgr_HAL_Shutdown(void* ctx) { }
void* TimeMgr_HAL_Init(void* config) { return 0; }
void TimeMgr_HAL_Shutdown(void* ctx) { }
void* PackageMgr_HAL_Init(void* config) { return 0; }
void PackageMgr_HAL_Shutdown(void* ctx) { }
void* AppleEventMgr_HAL_Init(void* config) { return 0; }
void AppleEventMgr_HAL_Shutdown(void* ctx) { }

