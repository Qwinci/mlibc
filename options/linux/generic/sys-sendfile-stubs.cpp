
#include <sys/sendfile.h>
#include <bits/ensure.h>
#include <syscall.h>

ssize_t sendfile(int out, int in, off_t *offset, size_t count) {
	// qwinci todo
	return syscall(SYS_sendfile, out, in, offset, count);
}

