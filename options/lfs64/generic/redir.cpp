#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <stdlib.h>

extern "C" FILE* fopen64(const char *__restrict filename, const char *__restrict mode) {
    return fopen(filename, mode);
}

extern "C" int fseeko64(FILE *stream, off64_t offset, int whence) {
    return fseeko(stream, offset, whence);
}

extern "C" off64_t ftello64(FILE *stream) {
    return ftello(stream);
}

extern "C" int __xstat(int version, const char *__restrict filename, struct stat *s) {
    return stat(filename, s);
}

extern "C" int __fxstat64(int version, int fd, struct stat *s) {
    return fstat(fd, s);
}

extern "C" int __fxstat(int version, int fd, struct stat *s) {
    return fstat(fd, s);
}

extern "C" int __lxstat(int version, const char *__restrict filename, struct stat *__restrict s) {
    return lstat(filename, s);
}

extern "C" int stat64(const char *__restrict filename, struct stat *__restrict s) {
    return stat(filename, s);
}

extern "C" int scandir64(const char *path, struct dirent ***res, int (*select)(const struct dirent *),
		int (*compare)(const struct dirent **, const struct dirent **)) {
    return scandir(path, res, select, compare);
}

extern "C" int fstat64(int fd, struct stat *s) {
    return fstat(fd, s);
}

extern "C" int lstat64(const char *__restrict filename, struct stat *__restrict s) {
    return lstat(filename, s);
}

extern "C" struct dirent *readdir64(DIR *dir) {
    return readdir(dir);
}

extern "C" int setrlimit64(int resource, const struct rlimit *rlim) {
    return setrlimit(resource, rlim);
}

extern "C" int getrlimit64(int resource, struct rlimit *rlim) {
    return getrlimit(resource, rlim);
}

extern "C" void *mmap64(void *ptr, size_t size, int prot, int flags, int fd, off64_t offset) {
    return mmap(ptr, size, prot, flags, fd, offset);
}

extern "C" int ftruncate64(int fd, off64_t size) {
    return ftruncate(fd, size);
}

extern "C" int statfs64(const char *__restrict path, struct statfs *s) {
    return statfs(path, s);
}

extern "C" int fstatfs64(int fd, struct statfs *s) {
    return fstatfs(fd, s);
}

extern "C" int mkostemp64(char *path, int flags) {
    return mkostemp(path, flags);
}

extern "C" int fstatat64(int fd, const char* __restrict path, struct stat *__restrict s, int flags) {
    return fstatat(fd, path, s, flags);
}

extern "C" int fstatvfs64(int fd, struct statvfs *s) {
    return fstatvfs(fd, s);
}

extern "C" ssize_t pwrite64(int fd, const void *buf, size_t n, off64_t off) {
    return pwrite(fd, buf, n, off);
}

extern "C" int posix_fallocate64(int fd, off64_t offset, off64_t size) {
    return posix_fallocate(fd, offset, size);
}
