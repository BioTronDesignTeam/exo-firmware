#include <sys/stat.h>
#include <errno.h>

__attribute__((weak)) int _close(int file) { (void)file; return -1; }
__attribute__((weak)) int _fstat(int file, struct stat *st) { (void)file; st->st_mode = S_IFCHR; return 0; }
__attribute__((weak)) int _isatty(int file) { (void)file; return 1; }
__attribute__((weak)) int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
__attribute__((weak)) int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
__attribute__((weak)) int _write(int file, char *ptr, int len) { (void)file; (void)ptr; return len; }
__attribute__((weak)) void _exit(int status) { (void)status; while(1); }
__attribute__((weak)) int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
__attribute__((weak)) int _getpid(void) { return 1; }
