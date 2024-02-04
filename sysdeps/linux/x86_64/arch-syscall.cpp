#include <sys/syscall.h>
#include <bits/syscall.h>

using sc_word_t = __sc_word_t;

sc_word_t __do_syscall0(long sc) {
	sc_word_t ret;
	asm volatile ("syscall" : "=a"(ret)
			: "a"(sc)
			: "rcx", "r11", "memory");
	return ret;
}

sc_word_t __do_syscall1(long sc,
		sc_word_t arg1) {
	sc_word_t ret;
	asm volatile ("syscall" : "=a"(ret)
			: "a"(sc), "D"(arg1)
			: "rcx", "r11", "memory");
	return ret;
}

sc_word_t __do_syscall2(long sc,
		sc_word_t arg1, sc_word_t arg2) {
	sc_word_t ret;
	asm volatile ("syscall" : "=a"(ret)
			: "a"(sc), "D"(arg1), "S"(arg2)
			: "rcx", "r11", "memory");
	return ret;
}

sc_word_t __do_syscall3(long sc,
		sc_word_t arg1, sc_word_t arg2, sc_word_t arg3) {
	sc_word_t ret;
	asm volatile ("syscall" : "=a"(ret)
			: "a"(sc), "D"(arg1), "S"(arg2), "d"(arg3)
			: "rcx", "r11", "memory");
	return ret;
}

sc_word_t __do_syscall4(long sc,
		sc_word_t arg1, sc_word_t arg2, sc_word_t arg3,
		sc_word_t arg4) {
	sc_word_t ret;
	register sc_word_t arg4_reg asm("r10") = arg4;
	asm volatile ("syscall" : "=a"(ret)
			: "a"(sc), "D"(arg1), "S"(arg2), "d"(arg3),
				"r"(arg4_reg)
			: "rcx", "r11", "memory");
	return ret;
}

sc_word_t __do_syscall5(long sc,
		sc_word_t arg1, sc_word_t arg2, sc_word_t arg3,
		sc_word_t arg4, sc_word_t arg5) {
	sc_word_t ret;
	register sc_word_t arg4_reg asm("r10") = arg4;
	register sc_word_t arg5_reg asm("r8") = arg5;
	asm volatile ("syscall" : "=a"(ret)
			: "a"(sc), "D"(arg1), "S"(arg2), "d"(arg3),
				"r"(arg4_reg), "r"(arg5_reg)
			: "rcx", "r11", "memory");
	return ret;
}

sc_word_t __do_syscall6(long sc,
		sc_word_t arg1, sc_word_t arg2, sc_word_t arg3,
		sc_word_t arg4, sc_word_t arg5, sc_word_t arg6) {
	sc_word_t ret;
	register sc_word_t arg4_reg asm("r10") = arg4;
	register sc_word_t arg5_reg asm("r8") = arg5;
	register sc_word_t arg6_reg asm("r9") = arg6;
	asm volatile ("syscall" : "=a"(ret)
			: "a"(sc), "D"(arg1), "S"(arg2), "d"(arg3),
				"r"(arg4_reg), "r"(arg5_reg), "r"(arg6_reg)
			: "rcx", "r11", "memory");
	return ret;
}

#if !MLIBC_BUILDING_RTDL
#include <errno.h>
extern "C" int __syscall_error(sc_word_t v) {
	if(static_cast<unsigned long>(v) > -4096UL) {
		errno = -v;
		return -1;
	}
	return 0;
}

asm(
	".globl syscall\n"
	".type syscall, @function\n"
	"syscall:\n"
	"mov %rdi, %rax;"
	"mov %rsi, %rdi;"
	"mov %rdx, %rsi;"
	"mov %rcx, %rdx;"
	"mov %r8, %r10;"
	"mov %r9, %r8;"
	"mov 8(%rsp), %r9;"
	"syscall;"
	"cmpq $-4095, %rax;"
	"jae 1f;"
	"ret;"
	"1:\n"
	"call __syscall_error;"
	"ret"
);
#endif
