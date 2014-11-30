###########################################################
#
# software division implementations for arm7tdmi processor
#
###########################################################

	.global __aeabi_idiv
__aeabi_idiv:
	cmp	r1, #0
	beq	.divsi3_skip_div0_test_g

	.global .divsi3_skip_div0_test
.divsi3_skip_div0_test:
	eor	ip, r0, r1
 	rsbmi	r1, r1, #0
 	subs	r2, r1, #1
 	beq	.divsi3_skip_div0_test_a
 	movs	r3, r0
 	rsbmi	r3, r0, #0
 	cmp	r3, r1
 	bls	.divsi3_skip_div0_test_b
 	tst	r1, r2
 	beq	.divsi3_skip_div0_test_c
 	tst	r1, #-536870912
 	lsleq	r1, r1, #3
 	moveq	r2, #8
 	movne	r2, #1
.divsi3_skip_div0_test_d:
 	cmp	r1, #268435456
 	cmpcc	r1, r3
 	lslcc	r1, r1, #4
 	lslcc	r2, r2, #4
 	bcc	.divsi3_skip_div0_test_d
.divsi3_skip_div0_test_e:
 	cmp	r1, #-2147483648
 	cmpcc	r1, r3
 	lslcc	r1, r1, #1
 	lslcc	r2, r2, #1
 	bcc	.divsi3_skip_div0_test_e
 	mov	r0, #0
.divsi3_skip_div0_test_f:
 	cmp	r3, r1
 	subcs	r3, r3, r1
 	orrcs	r0, r0, r2
 	cmp	r3, r1, lsr #1
 	subcs	r3, r3, r1, lsr #1
 	orrcs	r0, r0, r2, lsr #1
 	cmp	r3, r1, lsr #2
 	subcs	r3, r3, r1, lsr #2
 	orrcs	r0, r0, r2, lsr #2
 	cmp	r3, r1, lsr #3
 	subcs	r3, r3, r1, lsr #3
 	orrcs	r0, r0, r2, lsr #3
 	cmp	r3, #0
 	lsrhi	r2, r2, #4
 	lsrne	r1, r1, #4
 	bne	.divsi3_skip_div0_test_f
 	cmp	ip, #0
 	rsbmi	r0, r0, #0
 	bx	lr
.divsi3_skip_div0_test_a:
 	teq	ip, r0
 	rsbmi	r0, r0, #0
 	bx	lr
.divsi3_skip_div0_test_b:
 	movcc	r0, #0
 	asreq	r0, ip, #31
 	orreq	r0, r0, #1
 	bx	lr
.divsi3_skip_div0_test_c:
 	cmp	r1, #65536
 	lsrcs	r1, r1, #16
 	movcs	r2, #16
 	movcc	r2, #0
 	cmp	r1, #256
 	lsrcs	r1, r1, #8
 	addcs	r2, r2, #8
 	cmp	r1, #16
 	lsrcs	r1, r1, #4
 	addcs	r2, r2, #4
 	cmp	r1, #4
 	addhi	r2, r2, #3
 	addls	r2, r2, r1, lsr #1
 	cmp	ip, #0
 	lsr	r0, r3, r2
 	rsbmi	r0, r0, #0
 	bx	lr
.divsi3_skip_div0_test_g:
 	cmp	r0, #0
 	mvngt	r0, #-2147483648
 	movlt	r0, #-2147483648
 	b	__aeabi_idiv0

	.global __aeabi_idivmod
__aeabi_idivmod:
 	cmp	r1, #0
 	beq	.divsi3_skip_div0_test_g
 	push	{r0, r1, lr}
 	bl	.divsi3_skip_div0_test
 	pop	{r1, r2, lr}
 	mul	r3, r2, r0
 	sub	r1, r1, r3
 	bx	lr

	.global __aeabi_idiv0
__aeabi_idiv0:
 	bx	lr

        .global __aeabi_uidiv
__aeabi_uidiv:
        subs	r2, r1, #1
        bxeq	lr
        bcc .udiv_step_3
        cmp	r0, r1
        bls	.udiv_step_1
        tst	r1, r2
        beq	.udiv_step_2
        clz	r3, r0
        clz	r2, r1
        sub	r3, r2, r3
        rsbs	r3, r3, #31
        addne	r3, r3, r3, lsl #1
        mov	r2, #0
        addne	pc, pc, r3, lsl #2
        nop
        cmp	r0, r1, lsl #31
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #31
        cmp	r0, r1, lsl #30
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #30
        cmp	r0, r1, lsl #29
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #29
        cmp	r0, r1, lsl #28
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #28
        cmp	r0, r1, lsl #27
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #27
        cmp	r0, r1, lsl #26
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #26
        cmp	r0, r1, lsl #25
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #25
        cmp	r0, r1, lsl #24
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #24
        cmp	r0, r1, lsl #23
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #23
        cmp	r0, r1, lsl #22
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #22
        cmp	r0, r1, lsl #21
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #21
        cmp	r0, r1, lsl #20
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #20
        cmp	r0, r1, lsl #19
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #19
        cmp	r0, r1, lsl #18
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #18
        cmp	r0, r1, lsl #17
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #17
        cmp	r0, r1, lsl #16
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #16
        cmp	r0, r1, lsl #15
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #15
        cmp	r0, r1, lsl #14
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #14
        cmp	r0, r1, lsl #13
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #13
        cmp	r0, r1, lsl #12
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #12
        cmp	r0, r1, lsl #11
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #11
        cmp	r0, r1, lsl #10
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #10
        cmp	r0, r1, lsl #9
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #9
        cmp	r0, r1, lsl #8
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #8
        cmp	r0, r1, lsl #7
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #7
        cmp	r0, r1, lsl #6
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #6
        cmp	r0, r1, lsl #5
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #5
        cmp	r0, r1, lsl #4
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #4
        cmp	r0, r1, lsl #3
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #3
        cmp	r0, r1, lsl #2
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #2
        cmp	r0, r1, lsl #1
        adc	r2, r2, r2
        subcs	r0, r0, r1, lsl #1
        cmp	r0, r1
        adc	r2, r2, r2
        subcs	r0, r0, r1
        mov	r0, r2
        bx	lr
.udiv_step_1:
        moveq	r0, #1
        movne	r0, #0
        bx	lr
.udiv_step_2:
        clz	r2, r1
        rsb	r2, r2, #31
        lsr	r0, r0, r2
        bx	lr
.udiv_step_3:
        cmp	r0, #0
        mvnne	r0, #0
        b	__aeabi_idiv0

        .global __aeabi_uidivmod
__aeabi_uidivmod:
        cmp	r1, #0
        beq	.udiv_step_3
        push 	{r0, r1, lr}
        bl	__aeabi_uidiv
        pop	{r1, r2, lr}
        mul	r3, r2, r0
        sub	r1, r1, r3
        bx	lr
