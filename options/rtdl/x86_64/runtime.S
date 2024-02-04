.global __mlibcTlsdescStatic
.hidden __mlibcTlsdescStatic
.type __mlibcTlsdescStatic,@function
__mlibcTlsdescStatic:
	mov 8(%rax), %rax
	ret

// This function depends on the Tcb layout, since it pulls out the dtv pointer
// out of the thread control block
.global __mlibcTlsdescDynamic
.hidden __mlibcTlsdescDynamic
.type __mlibcTlsdescDynamic,@function
__mlibcTlsdescDynamic:
	ud2
	// stp x1, x2, [sp, #-16]!
	// ldr x0, [x0, #8]
	// ldp x1, x2, [x0] // tlsIndex, addend
	// mrs x0, tpidr_el0 // tp
	// ldr x0, [x0, #-96] // tp->dtvPointers
	// ldr x0, [x0, x1, lsl 3] // [tlsIndex]
	// add x0, x0, x2 // + addend
	// mrs x1, tpidr_el0 // tp
	// sub x0, x0, x1 // result - tp
	// ldp x1, x2, [sp], #16
	ret

.global pltRelocateStub
pltRelocateStub:
	# we need to save / restore all registers than can hold function arguments
	# we do not need to save callee-saved registers as they will not be trashed by lazyRelocate
	# TODO: save floating point argument registers

	push %rsi
	push %rdi
	mov 16(%rsp), %rdi
	mov 24(%rsp), %rsi

	push %rax
	push %rcx
	push %rdx
	push %r8
	push %r9
	push %r10

	call lazyRelocate
	mov %rax, %r11

	pop %r10
	pop %r9
	pop %r8
	pop %rdx
	pop %rcx
	pop %rax
	
	pop %rdi
	pop %rsi
	add $16, %rsp
	jmp *%r11

.section .note.GNU-stack,"",%progbits

