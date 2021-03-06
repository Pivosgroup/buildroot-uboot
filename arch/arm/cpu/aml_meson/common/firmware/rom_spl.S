/*
 * aml6236-m nand spl
 * @author jerry.yu
 *
 */
/* These define will be replace with */
#include <config.h>
#include <version.h>
#include <asm/arch/romboot.h>
.globl _start
_start: 
	b	reset
	ldr	pc,_undefined_instruction
	ldr	pc,_software_interrupt
	ldr	pc,_prefetch_abort
	ldr	pc,_data_abort
	ldr	pc,_not_used
	ldr	pc,_irq
	ldr	pc,_fiq
reset:
	
  ldr     sp, __ROM_STACK_END  // setup stack pointer 
	/*
		Whether I am running in SRAM 
      */
  adr r0, _start       		// r0 <- current position of code
  ldr r1, _AHB_SRAM_BASE  // test if we run from flash or RAM
  cmp r0, r1           		// dont reloc during debug
  beq sdram_running				// skip relocate 
  ldr r2,_READ_SIZE
  bl  ipl_memcpy
  add pc,r1,#( sdram_running - _start )
sdram_running:
	ldr r3,__BSS_START
	ldr r4,__BSS_END
	mov r5,#0
clr_bss:
	cmp r3,r4
	beq clr_bss_end
	str r5,[r3]
	add r3,r3,#4
	b 	clr_bss
clr_bss_end:

	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	mcr	p15, 0, r0, c1, c0, 0
	adr r0, _start
  mcr p15,0,r0,c12,c0,0
  bl	lowlevel_init
#ifndef AML_DEBUG_ROM
  ldr   r0,__TEXT_BASE
  ldr   r1,__TEXT_SIZE
  bl	relocate_init
  ldr   pc,__TEXT_BASE
#endif  

_AHB_SRAM_BASE:          //在<soc/romboot.h> 定义
  .word AHB_SRAM_BASE
_READ_SIZE:
	.word READ_SIZE
_AHB_SRAM_END:
  .word AHB_SRAM_BASE + READ_SIZE
__TEXT_BASE:
  .word TEXT_BASE
__TEXT_SIZE:
  .word 0x60000 - READ_SIZE   //uboot.bin  尺寸
__ROM_STACK_END:
  .word ROM_STACK_END
__BSS_START:
	.word _bssstart
__BSS_END:
	.word _bssend
  .global	romboot_info
  .type   romboot_info, %object
	.size   romboot_info, 4
romboot_info:
        .word   C_ROM_BOOT_DEBUG
.global	magic_info
  .type   magic_info, %object
	.size   magic_info, 4
magic_info:
	.word  __magic_word
#if 1	        
	.global	ipl_memcpy
	.type	ipl_memcpy, %function
ipl_memcpy:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	str	r4, [sp, #-4]!
	movs	r2, r2, lsr #2
	beq	.L4
	mov	ip, #0
	mov	r3, ip
.L3:
	mov	ip, ip, asl #2
	ldr	r4, [r1, ip]
	str	r4, [r0, ip]
	add	r3, r3, #1
	mov	ip, r3
	cmp	r3, r2
	bcc	.L3
.L4:
	ldmfd	sp!, {r4}
	bx	lr
#endif	
.global wait
wait:
	ldr pc,target_addr
target_addr:
	.word  0x49001ff4
test_func:
	dmb
	isb
	ldr pc,target_addr
.global _end_wait
_end_wait:
.global _end_rom_spl
_end_rom_spl:
undefined_instruction: 
	mov r0,#1
	b fail_sleep
software_interrupt:	
	mov r0,#2
	b fail_sleep
prefetch_abort:	
	mov r0,#3
	b fail_sleep
data_abort:
	mov r0,#4
	b fail_sleep		
	
not_used:		
	mov r0,#5
	b fail_sleep
irq:	
	mov r0,#6
	b fail_sleep		
fiq:
	mov r0,#7
	b fail_sleep	
fail_sleep:
	ldr     sp, =0x49002000  // setup stack pointer 			
	bl do_exception
	wfi

_undefined_instruction: .word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq
_pad:			.word 0x12345678 /* now 16*4=64 */
