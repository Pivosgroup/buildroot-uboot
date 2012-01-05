#include <asm/arch/am_regs.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gpio.h>

int board_ver = 0;

void power_hold(void)
{   
    //power hold
    //printf("power hold\n");

	clear_mio_mux(12, 0x40);		//disable ENC_6
	clear_mio_mux(0, 0x100);		//disable TCON_CPH3
	clear_mio_mux(2, 0x40000000);		//disable PWM_B
	clear_mio_mux(2, 0x10000000);		//disable DEMOD_RF_PWM
	clear_mio_mux(0, 0x200000);		//disable LED_BL_PWM
	clear_mio_mux(9, 0x800000);		//disable WIFI Debug

	set_gpio_mode(GPIOA_bank_bit(8), GPIOA_bit_bit0_14(8), GPIO_OUTPUT_MODE);   // OEN = 0 (output) 
	set_gpio_val(GPIOA_bank_bit(8), GPIOA_bit_bit0_14(8), 1);   // Output level = 1 (pull high)
    //*(volatile unsigned long *)(0xC1108030) &= ~(1 << 12);              // OEN = 0 (output) 
    //*(volatile unsigned long *)(0xC1108034) |= (1 << 12);               // Output level = 1 (pull high)
}
void power_unhold(void)
{   
    //power unhold
    //printf("power hold\n");

	clear_mio_mux(12, 0x40);		//disable ENC_6
	clear_mio_mux(0, 0x100);		//disable TCON_CPH3
	clear_mio_mux(2, 0x40000000);		//disable PWM_B
	clear_mio_mux(2, 0x10000000);		//disable DEMOD_RF_PWM
	clear_mio_mux(0, 0x200000);		//disable LED_BL_PWM
	clear_mio_mux(9, 0x800000);		//disable WIFI Debug

	set_gpio_mode(GPIOA_bank_bit(8), GPIOA_bit_bit0_14(8), GPIO_OUTPUT_MODE);   // OEN = 0 (output) 
	set_gpio_val(GPIOA_bank_bit(8), GPIOA_bit_bit0_14(8), 0);   // Output level = 0 (pull high)
    //*(volatile unsigned long *)(0xC1108030) &= ~(1 << 12);              // OEN = 0 (output) 
    //*(volatile unsigned long *)(0xC1108034) &= ~(1 << 12);               // Output level = 0 (pull low)
}

static int powerkey_init(void)
{
    if(board_ver == 0){
        WRITE_CBUS_REG(0x21d0/*RTC_ADDR0*/, (READ_CBUS_REG(0x21d0/*RTC_ADDR0*/) &~(1<<11)));
        WRITE_CBUS_REG(0x21d1/*RTC_ADDR0*/, (READ_CBUS_REG(0x21d1/*RTC_ADDR0*/) &~(1<<3)));
    }  
    return 1;
}
static int powerkey_scan(void)
{
    if(board_ver == 0)
        return ((READ_CBUS_REG(0x21d1/*RTC_ADDR1*/) >> 2) & 1) ? 0 : 1;
    else
        return (READ_CBUS_REG(ASSIST_HW_REV)&(1<<10))? 0:1;  //GP_INPUT2  bit 10
}

int powerkey_hold(void)
{
    unsigned long hold_time = 100000, polling_time = 10000, tmp;

    powerkey_init();

    while(hold_time > 0)
    {
        udelay(polling_time);
        tmp = powerkey_scan();
        printf("powerkey: %d\n", tmp);
        if(!tmp)  break;
        hold_time -= polling_time;
    }
    if(hold_time > 0)
    {
        power_unhold();
        printf("Power Down!\n");
        hang();
    }
    else
    {
        power_hold();
        printf("Power Up!\n");
    }
}