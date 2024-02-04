#ifndef MLIBC_LOCALE
#define MLIBC_LOCALE

#include <bits/posix/locale_t.h>
#include <bits/nl_item.h>

namespace mlibc {

char *nl_langinfo(nl_item item);

} // namespace mlibc

#endif // MLIBC_LOCALE
