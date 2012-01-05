#include <asm/arch/firm/regs.h>
#include <asm/arch/firm/reg_addr.h>
#include <asm/arch/firm/timing.h>
#include <asm/arch/romboot.h>
#include <memtest.h>
#include <config.h>
#include <asm/arch/firm/io.h>

void lowlevel_init(void* cur,void * target)
{
        int i;
	writel((1<<22)|100000,P_WATCHDOG_TC);//enable Watchdog 1 seconds
	//Adjust 1us timer base
	WRITE_CBUS_REG_BITS(PREG_CTLREG0_ADDR,CONFIG_CRYSTAL_MHZ,4,5);
	/*
        Select TimerE 1 us base
        */
	clrsetbits_le32(P_ISA_TIMER_MUX,0x7<<8,0x1<<8);
#ifdef CONFIG_STV       // added by steven for uart rx pullup and led status.
        WRITE_CBUS_REG( PREG_FGPIO_O, READ_CBUS_REG(PREG_FGPIO_O) | (1<<13) );  // GPIOC_13 LED YELLOW ON
        WRITE_CBUS_REG( PREG_FGPIO_O, READ_CBUS_REG(PREG_FGPIO_O) | (1<<14) );  // GPIOC_14 LED BLUE ON
        WRITE_CBUS_REG( PREG_FGPIO_O, READ_CBUS_REG(PREG_FGPIO_O) | (1<<15) );  // GPIOC_15 LED RED ON
        WRITE_CBUS_REG( PREG_FGPIO_EN_N, READ_CBUS_REG(PREG_FGPIO_EN_N) & ~((1<<13)|(1<<14)|(1<<15)) ); 
#endif
    	memory_pll_init(0,NULL);
	serial_puts("\n\n\nFirmware start at: ");
	serial_put_dword(get_timer(0));
//	serial_put_dword(readl(0xc1100000+0x200b*4));
        __udelay(1000);//delay 1 ms , wait pll ready	
	
    	do{
    	}while(ddr_init_test());
	writel(0,P_WATCHDOG_TC);//disable Watchdog    
	    
#ifdef CONFIG_STV
	__udelay(1000);//delay 2 ms , wait ready	
#endif
#ifdef CONFIG_STV // added by steven for led status.
#ifndef CONFIG_RESCUE
        WRITE_CBUS_REG( PREG_FGPIO_O, READ_CBUS_REG(PREG_FGPIO_O) | (1<<13) );  // GPIOC_13 LED YELLOW ON
        WRITE_CBUS_REG( PREG_FGPIO_O, READ_CBUS_REG(PREG_FGPIO_O) | (1<<14) );  // GPIOC_14 LED BLUE ON
        WRITE_CBUS_REG( PREG_FGPIO_O, READ_CBUS_REG(PREG_FGPIO_O) & ~(1<<15) );  // GPIOC_15 LED RED OFF
        WRITE_CBUS_REG( PREG_FGPIO_EN_N, READ_CBUS_REG(PREG_FGPIO_EN_N) & ~((1<<13)|(1<<14)|(1<<15)) ); 
#endif
#endif
	serial_puts("\nFirmware started, now starting u-boot...\n");
}
