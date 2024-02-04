#include <string.h>

extern "C" char *__xpg_strerror_r(int num, char *buf, size_t buflen) {
    strerror_r(num, buf, buflen);
    return buf;
}

extern "C" char *strerror_l(int num, locale_t) {
    return strerror(num);
}

extern "C" char __libc_single_threaded = 0;
