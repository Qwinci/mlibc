#pragma once

#include <stdint.h>
#include <limits.h>
#include <bits/size_t.h>
#include <frg/array.hpp>

/*
 * Explanation of cancellation bits:
 *
 * tcbCancelEnableBit and tcbCancelAsyncBit should be self-explanatory,
 * they are set if cancellation is enabled, or asynchronous, respectively.
 *
 * tcbCancelTriggerBit is set whenever a cancellation is triggered, which is
 * in pthread_cancel() or in the signal handler. This bit is used by
 * pthread_testcancel() to check whether a cancellation has been requested,
 * and also by cancellable syscalls.
 *
 * tcbCancelingBit is set when a cancellation is currently being handled. This
 * is to avoid a situation in which a cancellation handler gets interrupted by
 * a SIGCANCEL and a second cancellation handler gets executed on top of the
 * previous one. Right now this cannot happen, since we stay in signal handler
 * context when canceling/exiting. In the future this might be done outside
 * of a signal handler, in which case we shouldn't restart the cancellation process.
 *
 * tcbExitingBit is set when the thread starts the exit procedure. Currently
 * this is just an exit, but in the future this will be a stack unwinding
 * procedure, which shouldn't be reentered. Not currently set anywhere,
 * may be done so in the future.
 *
 * TODO(geert): update this comment when we do unwinding in the exit procedure.
 */

namespace {
	// Set when the cancellation is enabled
	constexpr unsigned int tcbCancelEnableBit = 1 << 0;
	// 1 - cancellation is asynchronous, 0 - cancellation is deferred
	constexpr unsigned int tcbCancelAsyncBit = 1 << 1;
	// Set when the thread has been cancelled
	constexpr unsigned int tcbCancelTriggerBit = 1 << 2;
	// Set when the thread is in the process of being cancelled.
	constexpr unsigned int tcbCancelingBit = 1 << 3;
	// Set when the thread is exiting.
	constexpr unsigned int tcbExitingBit = 1 << 4;
}

namespace mlibc {
	// Returns true when bitmask indicates thread has been asynchronously
	// cancelled.
	static constexpr bool tcb_async_cancelled(int value) {
		return (value & (tcbCancelEnableBit | tcbCancelAsyncBit
				| tcbCancelTriggerBit)) == (tcbCancelEnableBit
					| tcbCancelAsyncBit | tcbCancelTriggerBit);
	}

	// Returns true when bitmask indicates async cancellation is enabled.
	static constexpr bool tcb_async_cancel(int value) {
		return (value & (tcbCancelEnableBit | tcbCancelAsyncBit))
			== (tcbCancelEnableBit | tcbCancelAsyncBit);
	}

	// Returns true when bitmask indicates cancellation is enabled.
	static constexpr bool tcb_cancel_enabled(int value) {
		return (value & tcbCancelEnableBit);
	}

	// Returns true when bitmask indicates threas has been cancelled.
	static constexpr bool tcb_cancelled(int value) {
		return (value & (tcbCancelEnableBit | tcbCancelTriggerBit))
		       == (tcbCancelEnableBit | tcbCancelTriggerBit);
	}

#if !MLIBC_STATIC_BUILD && !MLIBC_BUILDING_RTDL
	// In non-static builds, libc.so always has a TCB available.
	constexpr bool tcb_available_flag = true;
#else
	// Otherwise this will be set to true after RTDL has initialized the TCB.
	extern bool tcb_available_flag;
#endif
}

struct Tcb {
	Tcb *selfPointer;
	size_t dtvSize;
	void **dtvPointers;
	int tid;
	int didExit;
	void *returnValue;
	uintptr_t stackCanary;
	int cancelBits;
	struct AtforkHandler {
		void (*prepare)(void);
		void (*parent)(void);
		void (*child)(void);

		AtforkHandler *next;
		AtforkHandler *prev;
	};

	AtforkHandler *atforkBegin;
	AtforkHandler *atforkEnd;

	struct CleanupHandler {
		void (*func)(void *);
		void *arg;

		CleanupHandler *next;
		CleanupHandler *prev;
	};

	CleanupHandler *cleanupBegin;
	CleanupHandler *cleanupEnd;
	int isJoinable;

	struct LocalKey {
		void *value;
		uint64_t generation;
	};
	frg::array<LocalKey, PTHREAD_KEYS_MAX> *localKeys;

	size_t stackSize;
	void *stackAddr;
	size_t guardSize;
};

// There are a few places where we assume the layout of the TCB:
#if defined(__x86_64__)
// sysdeps/linux/x86_64/cp_syscall.S uses the offset of cancelBits.
// GCC also expects the stack canary to be at fs:0x28.
static_assert(offsetof(Tcb, stackCanary) == 0x28);
static_assert(offsetof(Tcb, cancelBits) == 0x30);
#elif defined(__aarch64__)
// The thread pointer on AArch64 points to 16 bytes before the end of the TCB.
// options/linker/aarch64/runtime.S uses the offset of dtvPointers.
static_assert(sizeof(Tcb) - offsetof(Tcb, dtvPointers) - 0x10 == 96);
// sysdeps/linux/aarch64/cp_syscall.S uses the offset of cancelBits.
static_assert(sizeof(Tcb) - offsetof(Tcb, cancelBits) - 0x10 == 64);
#elif defined(__riscv) && __riscv_xlen == 64
// The thread pointer on RISC-V points to *after* the TCB, and since
// we need to access specific fields that means that the value in
// sysdeps/linux/riscv64/cp_syscall.S needs to be updated whenever
// the struct is expanded.
static_assert(sizeof(Tcb) - offsetof(Tcb, cancelBits) == 80);
#else
#error "Missing architecture specific code."
#endif
