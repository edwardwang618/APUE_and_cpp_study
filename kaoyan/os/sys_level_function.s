	.arch armv8-a
	.file	"sys_level_function.c"
	.text
	.section	.rodata
	.align	3
.LC0:
	.string	"hello world"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB0:
	.cfi_startproc
	stp	x29, x30, [sp, -16]!
	.cfi_def_cfa_offset 16
	.cfi_offset 29, -16
	.cfi_offset 30, -8
	mov	x29, sp
	mov	x2, 13
	adrp	x0, .LC0
	add	x1, x0, :lo12:.LC0
	mov	w0, 1
	bl	write
	mov	w0, 0
	bl	_exit
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Debian 12.2.0-14+deb12u1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
