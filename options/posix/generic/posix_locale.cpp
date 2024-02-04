#include <bits/posix/posix_locale.h>
#include <bits/ensure.h>
#include <mlibc/debug.hpp>
#include <stdlib.h>

namespace {

bool newlocale_seen = false;
bool uselocale_seen = false;

}

struct __locale_struct {
	struct __locale_data *locales[13];
	const unsigned short *ctype_b;
	const int *ctype_tolower;
	const int *ctype_toupper;

	const char *names[13];
};

extern "C" int32_t * * __ctype_toupper_loc();
extern "C" int32_t * * __ctype_tolower_loc();
extern "C" const unsigned short **__ctype_b_loc(void);;

locale_t newlocale(int, const char *, locale_t) {
	// Due to all of the locale functions being stubs, the locale will not be used
	if(!newlocale_seen) {
		mlibc::infoLogger() << "mlibc: newlocale() is a no-op" << frg::endlog;
		newlocale_seen = true;
	}
	locale_t locale = static_cast<locale_t>(malloc(sizeof(__locale_struct)));
	locale->ctype_tolower = *__ctype_tolower_loc();
	locale->ctype_toupper = *__ctype_toupper_loc();
	locale->ctype_b = *__ctype_b_loc();
	return locale;
}

extern "C" [[gnu::alias("newlocale")]] locale_t __newlocale(int, const char *, locale_t);
extern "C" [[gnu::alias("freelocale")]] locale_t __freelocale(int, const char *, locale_t);
extern "C" [[gnu::alias("duplocale")]] locale_t __duplocale(int, const char *, locale_t);

void freelocale(locale_t locale) {
	free(locale);
}

locale_t uselocale(locale_t) {
	if(!uselocale_seen) {
		mlibc::infoLogger() << "mlibc: uselocale() is a no-op" << frg::endlog;
		uselocale_seen = true;
	}
	return nullptr;
}

extern "C" [[gnu::alias("uselocale")]] locale_t __uselocale(locale_t);

locale_t duplocale(locale_t old_locale) {
	locale_t new_locale = static_cast<locale_t>(malloc(sizeof(__locale_struct)));
	*new_locale = *old_locale;
	return new_locale;
}
