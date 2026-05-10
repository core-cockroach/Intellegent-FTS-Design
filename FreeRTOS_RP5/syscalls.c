/* syscalls.c – all stubs for newlib (no -specs) */
#include <sys/stat.h>
#include <errno.h>

/* Heap */
void *_sbrk(int incr) {
    static char *heap_end = (char *)0xF1000000;  // adjust as needed
    char *prev = heap_end;
    heap_end += incr;
    return (void *)prev;
}

/* Required syscalls */
int _close(int file) { (void)file; return -1; }
int _fstat(int file, struct stat *st) { (void)file; (void)st; return -1; }
int _isatty(int file) { (void)file; return 1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
int _write(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
int _getpid(void) { return 1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; return -1; }
void _exit(int status) { (void)status; while(1); }