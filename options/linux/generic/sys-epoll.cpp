
#include <errno.h>
#include <sys/epoll.h>

#include <bits/ensure.h>
#include <mlibc/linux-sysdeps.hpp>
#include <stddef.h>

int epoll_create(int) {
	int fd;
	MLIBC_CHECK_OR_ENOSYS(mlibc::sys_epoll_create, -1);
	if(int e = mlibc::sys_epoll_create(0, &fd); e) {
		errno = e;
		return -1;
	}
	return fd;
}

int epoll_pwait(int epfd, struct epoll_event *evnts, int n, int timeout,
		const sigset_t *sigmask) {
	int raised;
	MLIBC_CHECK_OR_ENOSYS(mlibc::sys_epoll_pwait, -1);
	if(int e = mlibc::sys_epoll_pwait(epfd, evnts, n, timeout, sigmask, &raised)) {
		errno = e;
		return -1;
	}
	return raised;
}

extern "C" int epoll_pwait2(int epfd, struct epoll_event *evnts, int n, const struct timespec *timeout,
               const sigset_t *sigmask) {
	int timeout_ms = timeout->tv_sec * 1000 + timeout->tv_nsec / 1000000L;
	return epoll_pwait(epfd, evnts, n, timeout_ms, sigmask);
}

int epoll_create1(int flags) {
	int fd;
	MLIBC_CHECK_OR_ENOSYS(mlibc::sys_epoll_create, -1);
	if(int e = mlibc::sys_epoll_create(flags, &fd); e) {
		errno = e;
		return -1;
	}
	return fd;
}

int epoll_ctl(int epfd, int mode, int fd, struct epoll_event *ev) {
	MLIBC_CHECK_OR_ENOSYS(mlibc::sys_epoll_ctl, -1);
	if(int e = mlibc::sys_epoll_ctl(epfd, mode, fd, ev); e) {
		errno = e;
		return -1;
	}
	return 0;
}

int epoll_wait(int epfd, struct epoll_event *evnts, int n, int timeout) {
	int raised;
	MLIBC_CHECK_OR_ENOSYS(mlibc::sys_epoll_pwait, -1);
	if(int e = mlibc::sys_epoll_pwait(epfd, evnts, n, timeout, NULL, &raised)) {
		errno = e;
		return -1;
	}
	return raised;
}

extern "C" int statx(int fd, const char *__restrict path, int flags, unsigned int mask, struct statx *__restrict buf) {
	errno = ENOSYS;
	return -1;
}

