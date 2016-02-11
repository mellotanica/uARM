###########################################################
#
# software division implementations for arm7tdmi processor
#
###########################################################

	.align	2
	.global	__udivmodsi4
	.type	__udivmodsi4, %function
__udivmodsi4:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #28
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	str	r2, [fp, #-24]
	mov	r3, #0
	str	r3, [fp, #-8]
	mov	r3, #1
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-20]
	cmp	r3, #0
	bne	.L4
	mov	r3, #0
	b	.L3
.L5:
	ldr	r3, [fp, #-20]
	mov	r3, r3, asl #1
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-12]
	mov	r3, r3, asl #1
	str	r3, [fp, #-12]
.L4:
	ldr	r3, [fp, #-20]
	cmp	r3, #0
	bge	.L5
	b	.L6
.L8:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #-16]
	cmp	r2, r3
	bhi	.L7
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-20]
	rsb	r3, r3, r2
	str	r3, [fp, #-16]
	ldr	r2, [fp, #-8]
	ldr	r3, [fp, #-12]
	add	r3, r2, r3
	str	r3, [fp, #-8]
.L7:
	ldr	r3, [fp, #-20]
	mov	r3, r3, lsr #1
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-12]
	mov	r3, r3, lsr #1
	str	r3, [fp, #-12]
.L6:
	ldr	r3, [fp, #-12]
	cmp	r3, #0
	bne	.L8
	ldr	r3, [fp, #-24]
	cmp	r3, #0
	beq	.L9
	ldr	r3, [fp, #-24]
	ldr	r2, [fp, #-16]
	str	r2, [r3]
.L9:
	ldr	r3, [fp, #-8]
.L3:
	mov	r0, r3
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	__udivmodsi4, .-__udivmodsi4
	.align	2
	.global	__aeabi_uidiv
	.type	__aeabi_uidiv, %function
__aeabi_uidiv:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	mov	r2, #0
	ldr	r1, [fp, #-12]
	ldr	r0, [fp, #-8]
	bl	__udivmodsi4(PLT)
	mov	r3, r0
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, lr}
	bx	lr
	.size	__aeabi_uidiv, .-__aeabi_uidiv
	.align	2
	.global	__aeabi_uidivmod
	.type	__aeabi_uidivmod, %function
__aeabi_uidivmod:
	stmfd	sp!, {r0, r1, ip, lr}
	bl	__aeabi_uidiv
	ldmfd	sp!, {r1, r2, ip, lr}
	mul	r3, r0, r2
	sub	r1, r1, r3
	mov	pc, lr
	.size	__aeabi_uidivmod, .-__aeabi_uidivmod
	.align	2
	.global	__aeabi_idiv
	.type	__aeabi_idiv, %function
__aeabi_idiv:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #16
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	mov	r3, #0
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-16]
	cmp	r3, #0
	bge	.L13
	ldr	r3, [fp, #-16]
	rsb	r3, r3, #0
	str	r3, [fp, #-16]
	mov	r3, #1
	str	r3, [fp, #-8]
.L13:
	ldr	r3, [fp, #-20]
	cmp	r3, #0
	bge	.L14
	ldr	r3, [fp, #-20]
	rsb	r3, r3, #0
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-8]
	eor	r3, r3, #1
	str	r3, [fp, #-8]
.L14:
	ldr	r3, [fp, #-16]
	ldr	r1, [fp, #-20]
	mov	r2, #0
	mov	r0, r3
	bl	__udivmodsi4(PLT)
	mov	r3, r0
	str	r3, [fp, #-12]
	ldr	r3, [fp, #-8]
	cmp	r3, #0
	beq	.L15
	ldr	r3, [fp, #-12]
	rsb	r3, r3, #0
	str	r3, [fp, #-12]
.L15:
	ldr	r3, [fp, #-12]
	mov	r0, r3
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, lr}
	bx	lr
	.size	__aeabi_idiv, .-__aeabi_idiv
	.align	2
	.global	__aeabi_idivmod
	.type	__aeabi_idivmod, %function
__aeabi_idivmod:
	stmfd	sp!, {r0, r1, ip, lr}
	bl	__aeabi_idiv
	ldmfd	sp!, {r1, r2, ip, lr}
	mul	r3, r0, r2
	sub	r1, r1, r3
	mov	pc, lr
	.size	__aeabi_idivmod, .-__aeabi_idivmod
