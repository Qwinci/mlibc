#include <resolv.h>
#include <bits/ensure.h>
#include <mlibc/debug.hpp>

int dn_expand(const unsigned char *, const unsigned char *,
		const unsigned char *, char *, int) {
	__ensure(!"Not implemented");
	__builtin_unreachable();
}

int res_query(const char *, int, int, unsigned char *, int) {
	__ensure(!"Not implemented");
	__builtin_unreachable();	
}

int res_init() {
	mlibc::infoLogger() << "mlibc: res_init is a stub!" << frg::endlog;
	return -1;
	return 0;
}

int res_ninit(res_state) {
	mlibc::infoLogger() << "mlibc: res_ninit is a stub!" << frg::endlog;
	return -1;
	return 0;
}

extern "C" int ns_initparse(const unsigned char *, int, void *msg) {
	return -1;
}

extern "C" int ns_parserr(void *msg, int sect, int, void *ns_rr_arg) {
	return -1;
}

extern "C" int res_nquery(res_state state, const char *dname, int klass, int type, unsigned char *answer, int anslen) {
	mlibc::infoLogger() << "mlibc: res_nquery is a stub!" << frg::endlog;
	return -1;
}

extern "C" [[gnu::alias("res_ninit")]] int __res_ninit(res_state);
extern "C" [[gnu::alias("res_nquery")]] int __res_nquery(res_state);
extern "C" [[gnu::alias("res_nclose")]] int __res_nclose(res_state);
extern "C" [[gnu::alias("dn_expand")]] int __dn_expand(res_state);

void res_nclose(res_state) {
	mlibc::infoLogger() << "mlibc: res_nclose is a stub!" << frg::endlog;
	return;
}

extern "C" int res_search(const char* dname, int klass, int type, unsigned char *answer, int anslen) {
	return -1;
}

/* This is completely unused, and exists purely to satisfy broken apps. */

struct __res_state *__res_state() {
	static struct __res_state res;
	return &res;
}
