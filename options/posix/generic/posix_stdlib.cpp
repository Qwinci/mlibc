
#include <abi-bits/fcntl.h>
#include <bits/ensure.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <frg/small_vector.hpp>
#include <mlibc/allocator.hpp>
#include <mlibc/debug.hpp>
#include <mlibc/posix-sysdeps.hpp>
#include <mlibc/rtdl-config.hpp>

namespace {
	constexpr bool debugPathResolution = false;
}

// Borrowed from musl
static uint32_t init[] = {
0x00000000,0x5851f42d,0xc0b18ccf,0xcbb5f646,
0xc7033129,0x30705b04,0x20fd5db4,0x9a8b7f78,
0x502959d8,0xab894868,0x6c0356a7,0x88cdb7ff,
0xb477d43f,0x70a3a52b,0xa8e4baf1,0xfd8341fc,
0x8ae16fd9,0x742d2f7a,0x0d1f0796,0x76035e09,
0x40f7702c,0x6fa72ca5,0xaaa84157,0x58a0df74,
0xc74a0364,0xae533cc4,0x04185faf,0x6de3b115,
0x0cab8628,0xf043bfa4,0x398150e9,0x37521657};

static int n = 31;
static int i = 3;
static int j = 0;
static uint32_t *x = init + 1;


static uint32_t lcg31(uint32_t x) {
	return (1103515245 * x + 12345) & 0x7fffffff;
}

static uint64_t lcg64(uint64_t x) {
	return 6364136223846793005ull * x + 1;
}

static void *savestate(void) {
	x[-1] = (n << 16) | (i << 8) | j;
	return x - 1;
}

static void loadstate(uint32_t *state) {
	x = state + 1;
	n = x[-1] >> 16;
	i = (x[-1] >> 8) & 0xff;
	j = x[-1] & 0xff;
}

long random(void) {
	long k;

	if(n == 0) {
		k = x[0] = lcg31(x[0]);
		return k;
	}
	x[i] += x[j];
	k = x[i] >> 1;
	if(++i == n)
		i = 0;
	if(++j == n)
		j = 0;

	return k;
}

double drand48(void) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

void srand48(long int) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

// Borrowed from musl
void srandom(unsigned int seed) {
	int k;
	uint64_t s = seed;

	if(n == 0) {
		x[0] = s;
		return;
	}
	i = n == 31 || n == 7 ? 3 : 1;
	j = 0;
	for(k = 0; k < n; k++) {
		s = lcg64(s);
		x[k] = s >> 32;
	}
	// Make sure x contains at least one odd number
	x[0] |= 1;
}

char *initstate(unsigned int seed, char *state, size_t size) {
	void *old;

	if(size < 8)
		return 0;
	old = savestate();
	if(size < 32)
		n = 0;
	else if(size < 64)
		n = 7;
	else if(size < 128)
		n = 15;
	else if(size < 256)
		n = 31;
	else
		n = 63;
	x = (uint32_t *)state + 1;
	srandom(seed);
	savestate();
	return (char *)old;
}

struct random_data {
    int32_t *fptr;		/* Front pointer.  */
    int32_t *rptr;		/* Rear pointer.  */
    int32_t *state;		/* Array of state values.  */
    int rand_type;		/* Type of random number generator.  */
    int rand_deg;		/* Degree of random number generator.  */
    int rand_sep;		/* Distance between front and rear.  */
    int32_t *end_ptr;		/* Pointer behind state table.  */
};

/* Linear congruential.  */
#define	TYPE_0		0
#define	BREAK_0		8
#define	DEG_0		0
#define	SEP_0		0
/* x**7 + x**3 + 1.  */
#define	TYPE_1		1
#define	BREAK_1		32
#define	DEG_1		7
#define	SEP_1		3
/* x**15 + x + 1.  */
#define	TYPE_2		2
#define	BREAK_2		64
#define	DEG_2		15
#define	SEP_2		1
/* x**31 + x**3 + 1.  */
#define	TYPE_3		3
#define	BREAK_3		128
#define	DEG_3		31
#define	SEP_3		3
/* x**63 + x + 1.  */
#define	TYPE_4		4
#define	BREAK_4		256
#define	DEG_4		63
#define	SEP_4		1

#define	MAX_TYPES	5	/* Max number of types above.  */
struct random_poly_info
{
  int seps[MAX_TYPES];
  int degrees[MAX_TYPES];
};
static const struct random_poly_info random_poly_info =
{
  { SEP_0, SEP_1, SEP_2, SEP_3, SEP_4 },
  { DEG_0, DEG_1, DEG_2, DEG_3, DEG_4 }
};

extern "C" int random_r(random_data *buf, int32_t *result);

extern "C" int srandom_r(unsigned int seed, struct random_data *buf) {
  int type;
  int32_t *state;
  long int i;
  int32_t word;
  int32_t *dst;
  int kc;
  if (buf == NULL)
    goto fail;
  type = buf->rand_type;
  if ((unsigned int) type >= MAX_TYPES)
    goto fail;
  state = buf->state;
  /* We must make sure the seed is not 0.  Take arbitrarily 1 in this case.  */
  if (seed == 0)
    seed = 1;
  state[0] = seed;
  if (type == TYPE_0)
    goto done;
  dst = state;
  word = seed;
  kc = buf->rand_deg;
  for (i = 1; i < kc; ++i)
    {
      /* This does:
	   state[i] = (16807 * state[i - 1]) % 2147483647;
	 but avoids overflowing 31 bits.  */
      long int hi = word / 127773;
      long int lo = word % 127773;
      word = 16807 * lo - 2836 * hi;
      if (word < 0)
	word += 2147483647;
      *++dst = word;
    }
  buf->fptr = &state[buf->rand_sep];
  buf->rptr = &state[0];
  kc *= 10;
  while (--kc >= 0)
    {
      int32_t discard;
      (void) random_r (buf, &discard);
    }
 done:
  return 0;
 fail:
  return -1;
}

extern "C" int initstate_r(unsigned int seed, char *arg_state, size_t n, struct random_data *buf) {
 if (buf == NULL) {
	errno = EINVAL;
 	 return -1;
 }
  int32_t *old_state = buf->state;
  if (old_state != NULL)
    {
      int old_type = buf->rand_type;
      if (old_type == TYPE_0)
	old_state[-1] = TYPE_0;
      else
	old_state[-1] = (MAX_TYPES * (buf->rptr - old_state)) + old_type;
    }
  int type;
  if (n >= BREAK_3)
    type = n < BREAK_4 ? TYPE_3 : TYPE_4;
  else if (n < BREAK_1)
    {
      if (n < BREAK_0) {
		errno = EINVAL;
  		return -1;
	  }
      type = TYPE_0;
    }
  else
    type = n < BREAK_2 ? TYPE_1 : TYPE_2;
  int degree = random_poly_info.degrees[type];
  int separation = random_poly_info.seps[type];
  buf->rand_type = type;
  buf->rand_sep = separation;
  buf->rand_deg = degree;
  int32_t *state = &((int32_t *) arg_state)[1];	/* First location.  */
  /* Must set END_PTR before srandom.  */
  buf->end_ptr = &state[degree];
  buf->state = state;
  srandom_r(seed, buf);
  state[-1] = TYPE_0;
  if (type != TYPE_0)
    state[-1] = (buf->rptr - state) * MAX_TYPES + type;
  return 0;
}

extern "C" int random_r(random_data *buf, int32_t *result) {
  int32_t *state;
  if (buf == NULL || result == NULL)
    goto fail;
  state = buf->state;
  if (buf->rand_type == TYPE_0)
    {
      int32_t val = ((state[0] * 1103515245U) + 12345U) & 0x7fffffff;
      state[0] = val;
      *result = val;
    }
  else
    {
      int32_t *fptr = buf->fptr;
      int32_t *rptr = buf->rptr;
      int32_t *end_ptr = buf->end_ptr;
      uint32_t val;
      val = *fptr += (uint32_t) *rptr;
      /* Chucking least random bit.  */
      *result = val >> 1;
      ++fptr;
      if (fptr >= end_ptr)
	{
	  fptr = state;
	  ++rptr;
	}
      else
	{
	  ++rptr;
	  if (rptr >= end_ptr)
	    rptr = state;
	}
      buf->fptr = fptr;
      buf->rptr = rptr;
    }
  return 0;
fail:
  errno = EINVAL;
  return -1;
}

char *setstate(char *state) {
	void *old;

	old = savestate();
	loadstate((uint32_t *)state);
	return (char *)old;
}

// ----------------------------------------------------------------------------
// Path handling.
// ----------------------------------------------------------------------------

int mkostemp(char *pattern, int flags) {
	flags &= ~O_WRONLY;
	auto n = strlen(pattern);
	__ensure(n >= 6);
	if(n < 6) {
		errno = EINVAL;
		return -1;
	}
	for(size_t i = 0; i < 6; i++) {
		if(pattern[n - 6 + i] == 'X')
			continue;
		errno = EINVAL;
		return -1;
	}

	// TODO: Do an exponential search.
	for(size_t i = 0; i < 999999; i++) {
		__ensure(sprintf(pattern + (n - 6), "%06zu", i) == 6);
//		mlibc::infoLogger() << "mlibc: mkstemp candidate is "
//				<< (const char *)pattern << frg::endlog;

		int fd;
		if(int e = mlibc::sys_open(pattern, O_RDWR | O_CREAT | O_EXCL | flags, S_IRUSR | S_IWUSR, &fd); !e) {
			return fd;
		}else if(e != EEXIST) {
			errno = e;
			return -1;
		}
	}

	errno = EEXIST;
	return -1;
}

int mkstemp(char *path) {
	return mkostemp(path, 0);
}

extern "C" int mkstemp64(char *path) {
	return mkstemp(path);
}

int mkostemps(char *pattern, int suffixlen, int flags) {
	(void)suffixlen;
	mlibc::infoLogger() << "mlibc: mkostemps ignores suffixlen!" << frg::endlog;
	return mkostemp(pattern, flags);
}

int mkstemps(char *pattern, int suffixlen) {
	return mkostemps(pattern, suffixlen, 0);
}

char *mkdtemp(char *pattern) {
	mlibc::infoLogger() << "mlibc mkdtemp(" << pattern << ") called" << frg::endlog;
	auto n = strlen(pattern);
	__ensure(n >= 6);
	if(n < 6) {
		errno = EINVAL;
		return NULL;
	}
	for(size_t i = 0; i < 6; i++) {
		if(pattern[n - 6 + i] == 'X')
			continue;
		errno = EINVAL;
		return NULL;
	}

	// TODO: Do an exponential search.
	for(size_t i = 0; i < 999999; i++) {
		__ensure(sprintf(pattern + (n - 6), "%06zu", i) == 6);
		if(int e = mlibc::sys_mkdir(pattern, S_IRWXU); !e) {
			return pattern;
		}else if(e != EEXIST) {
			errno = e;
			return NULL;
		}
	}

	errno = EEXIST;
	return NULL;
}

char *realpath(const char *path, char *out) {
	if(debugPathResolution)
		mlibc::infoLogger() << "mlibc realpath(): Called on '" << path << "'" << frg::endlog;
	frg::string_view path_view{path};

	// In case of the root, the string only contains the null-terminator.
	frg::small_vector<char, PATH_MAX, MemoryAllocator> resolv{getAllocator()};
	size_t ps;

	// If the path is relative, we have to preprend the working directory.
	if(path[0] == '/') {
		resolv.push_back(0);
		ps = 1;
	}else{
		MLIBC_CHECK_OR_ENOSYS(mlibc::sys_getcwd, nullptr);

		// Try to getcwd() until the buffer is large enough.
		resolv.resize(128);
		while(true) {
			int e = mlibc::sys_getcwd(resolv.data(), resolv.size());
			if(e == ERANGE) {
				resolv.resize(2 * resolv.size());
			}else if(!e) {
				break;
			}else{
				errno = e;
				return nullptr;
			}
		}
		frg::string_view cwd_view{resolv.data()};
		if(cwd_view == "/") {
			// Restore our invariant that we only store the null-terminator for the root.
			resolv.resize(1);
			resolv[0] = 0;
		}else{
			resolv.resize(cwd_view.size() + 1);
		}
		ps = 0;
	}

	// Contains unresolved links as a relative path compared to resolv.
	frg::small_vector<char, PATH_MAX, MemoryAllocator> lnk{getAllocator()};
	size_t ls = 0;

	auto process_segment = [&] (frg::string_view s_view) -> int {
		if(debugPathResolution)
			mlibc::infoLogger() << "mlibc realpath(): resolv is '" << resolv.data() << "'"
					<< ", segment is " << s_view.data()
					<< ", size: " << s_view.size() << frg::endlog;

		if(!s_view.size() || s_view == ".") {
			// Keep resolv invariant.
			return 0;
		}else if(s_view == "..") {
			// Remove a single segment from resolv.
			if(resolv.size() > 1) {
				auto slash = strrchr(resolv.data(), '/');
				__ensure(slash); // We never remove the leading sla.
				resolv.resize((slash - resolv.data()) + 1);
				*slash = 0; // Replace the slash by a null-terminator.
			}
			return 0;
		}

		// Append the segment to resolv.
		auto rsz = resolv.size();
		resolv[rsz - 1] = '/'; // Replace null-terminator by a slash.
		resolv.resize(rsz + s_view.size() + 1);
		memcpy(resolv.data() + rsz, s_view.data(), s_view.size());
		resolv[rsz + s_view.size()] = 0;

		// stat() the path to (1) see if it exists and (2) see if it is a link.
		if(!mlibc::sys_stat) {
			MLIBC_MISSING_SYSDEP();
			return ENOSYS;
		}
		if(debugPathResolution)
			mlibc::infoLogger() << "mlibc realpath(): stat()ing '"
					<< resolv.data() << "'" << frg::endlog;
		struct stat st;
		if(int e = mlibc::sys_stat(mlibc::fsfd_target::path,
				-1, resolv.data(), AT_SYMLINK_NOFOLLOW, &st); e)
			return e;

		if(S_ISLNK(st.st_mode)) {
			if(debugPathResolution) {
				mlibc::infoLogger() << "mlibc realpath(): Encountered symlink '"
					<< resolv.data() << "'" << frg::endlog;
			}

			if(!mlibc::sys_readlink) {
				MLIBC_MISSING_SYSDEP();
				return ENOSYS;
			}

			ssize_t sz = 0;
			char path[512];

			if (int e = mlibc::sys_readlink(resolv.data(), path, 512, &sz); e)
				return e;

			if(debugPathResolution) {
				mlibc::infoLogger() << "mlibc realpath(): Symlink resolves to '"
					<< frg::string_view{path, static_cast<size_t>(sz)} << "'" << frg::endlog;
			}

			if (path[0] == '/') {
				// Absolute path, replace resolv
				resolv.resize(sz);
				strncpy(resolv.data(), path, sz - 1);
				resolv.data()[sz - 1] = 0;

				if(debugPathResolution) {
					mlibc::infoLogger() << "mlibc realpath(): Symlink is absolute, resolv: '"
						<< resolv.data() << "'" << frg::endlog;
				}
			} else {
				// Relative path, revert changes to resolv, prepend to lnk
				resolv.resize(rsz);
				resolv[rsz - 1] = 0;

				auto lsz = lnk.size();
				lnk.resize((lsz - ls) + sz + 1);
				memmove(lnk.data() + sz, lnk.data() + ls, lsz - ls);
				memcpy(lnk.data(), path, sz);
				lnk[(lsz - ls) + sz] = 0;

				ls = 0;

				if(debugPathResolution) {
					mlibc::infoLogger() << "mlibc realpath(): Symlink is relative, resolv: '"
						<< resolv.data() << "' lnk: '"
						<< frg::string_view{lnk.data(), lnk.size()} << "'" << frg::endlog;
				}
			}
		}

		return 0;
	};

	// Each iteration of this outer loop consumes segment of the input path.
	// This design avoids copying the input path into lnk;
	// the latter could often involve additional allocations.
	while(ps < path_view.size()) {
		frg::string_view ps_view;
		if(auto slash = strchr(path + ps, '/'); slash) {
			ps_view = frg::string_view{path + ps, static_cast<size_t>(slash - (path + ps))};
		}else{
			ps_view = frg::string_view{path + ps, strlen(path) - ps};
		}
		ps += ps_view.size() + 1;

		// Handle one segment from the input path.
		if(int e = process_segment(ps_view); e) {
			errno = e;
			return nullptr;
		}

		// This inner loop consumes segments of lnk.
		while(ls < lnk.size()) {
			frg::string_view ls_view;
			if(auto slash = strchr(lnk.data() + ls, '/'); slash) {
				ls_view = frg::string_view{lnk.data() + ls, static_cast<size_t>(slash - (lnk.data() + ls))};
			}else{
				ls_view = frg::string_view{lnk.data() + ls, strlen(lnk.data()) - ls};
			}
			ls += ls_view.size() + 1;

			// Handle one segment from the link
			if(int e = process_segment(ls_view); e) {
				errno = e;
				return nullptr;
			}
		}

		// All of lnk was consumed, reset it
		lnk.resize(0);
		ls = 0;
	}

	if(resolv.size() == 1) {
		resolv.resize(0);
		resolv.push_back('/');
		resolv.push_back(0);
	}

	if(debugPathResolution)
		mlibc::infoLogger() << "mlibc realpath(): Returns '" << resolv.data() << "'" << frg::endlog;

	if(resolv.size() > PATH_MAX) {
		errno = ENAMETOOLONG;
		return nullptr;
	}

	if(!out)
		out = reinterpret_cast<char *>(getAllocator().allocate(resolv.size()));
	strcpy(out, resolv.data());
	return out;
}

extern "C" char *__realpath_chk(const char *path, char *resolved_path, size_t resolved_len) {
	return realpath(path, resolved_path);
}

// ----------------------------------------------------------------------------
// Pseudoterminals
// ----------------------------------------------------------------------------

int ptsname_r(int fd, char *buffer, size_t length) {
	auto sysdep = MLIBC_CHECK_OR_ENOSYS(mlibc::sys_ptsname, ENOSYS);

	if(int e = sysdep(fd, buffer, length); e)
		return e;

	return 0;
}

char *ptsname(int fd) {
	static char buffer[128];

	auto sysdep = MLIBC_CHECK_OR_ENOSYS(mlibc::sys_ptsname, NULL);

	if(int e = sysdep(fd, buffer, 128); e) {
		errno = e;
		return NULL;
	}

	return buffer;
}

int posix_openpt(int flags) {
	int fd;
	if(int e = mlibc::sys_open("/dev/ptmx", flags, 0, &fd); e) {
		errno = e;
		return -1;
	}

	return fd;
}

int unlockpt(int fd) {
	auto sysdep = MLIBC_CHECK_OR_ENOSYS(mlibc::sys_unlockpt, -1);

	if(int e = sysdep(fd); e) {
		errno = e;
		return -1;
	}

	return 0;
}

int grantpt(int) {
	return 0;
}

double strtod_l(const char *__restrict__ nptr, char ** __restrict__ endptr, locale_t) {
	mlibc::infoLogger() << "mlibc: strtod_l ignores locale!" << frg::endlog;
	return strtod(nptr, endptr);
}

long double strtold_l(const char *__restrict__, char ** __restrict__, locale_t) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

float strtof_l(const char *__restrict__ nptr, char **__restrict__ endptr, locale_t) {
	mlibc::infoLogger() << "mlibc: strtof_l ignores locales" << frg::endlog;
	return strtof(nptr, endptr);
}

extern "C" [[gnu::alias("strtof_l")]] float __strtof_l(const char *__restrict__ nptr, char **__restrict__ endptr, locale_t);
extern "C" [[gnu::alias("strtof_l")]] double __strtod_l(const char *__restrict__ nptr, char **__restrict__ endptr, locale_t);
extern "C" [[gnu::alias("strcoll_l")]] double __strcoll_l(const char *__restrict__ nptr, char **__restrict__ endptr, locale_t);

extern "C" size_t __strxfrm_l(char *__restrict dest, const char *__restrict src, size_t max_size, locale_t) {
	return strxfrm(dest, src, max_size);
}

extern "C" size_t __strftime_l(char *__restrict dest, size_t max_size, const char *__restrict format, const struct tm *__restrict ptr, locale_t) {
	return strftime(dest, max_size, format, ptr);
}

#include <wchar.h>

extern "C" int __wcscoll_l(const wchar_t *ws1, const wchar_t *ws2, locale_t) {
	return wcscoll(ws1, ws2);
}

extern "C" int __wcsxfrm_l(wchar_t *__restrict ws1, const wchar_t *__restrict ws2, size_t max_size, locale_t) {
	return wcsxfrm(ws1, ws2, max_size);
}

extern "C" size_t __wcsftime_l(wchar_t *__restrict dest, size_t max_size, const wchar_t *__restrict format, const struct tm *__restrict ptr, locale_t) {
	return wcsftime(dest, max_size, format, ptr);
}

int strcoll_l(const char *, const char *, locale_t) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

int getloadavg(double *, int) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

char *secure_getenv(const char *name) {
	if (mlibc::rtdlConfig().secureRequired)
		return NULL;
	else
		return getenv(name);
}

void *reallocarray(void *ptr, size_t m, size_t n) {
	if(n && m > -1 / n) {
		errno = ENOMEM;
		return 0;
	}

	return realloc(ptr, m * n);
}

char *canonicalize_file_name(const char *name) {
	return realpath(name, NULL);
}
