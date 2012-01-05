
#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <asm/arch/am_reg_addr.h>
#include <asm/arch/aml_card_sd.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/am_eth_reg.h>
#include <share_kernel.h>
#include <asm/arch/gpio.h>
/**
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
    /*
    @todo implement this function
    */
	DECLARE_GLOBAL_DATA_PTR;
	gd->bd->bi_arch_number=MACH_TYPE_MESON_8626M;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;

#ifdef CONFIG_CMD_KGDB

	kgdb_init();
#endif

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{
/*
    @todo implement this function
    */

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
    /*
    @todo implement this function
    */
}

#if defined(CONFIG_CMD_NET)
/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
    /*
    @todo implement this function
    */
	
}

extern int aml_eth_init(bd_t *bis);
int board_eth_init(bd_t *bis)
{
    /*
    @todo implement this function
    */
	///GPIOD15-24 for 8626M;
	///GPIOD12	nRst;
	///GPIOD13    n_int;
	printf("Set 8626m \n");
	//eth_clk_set(ETH_CLKSRC_SYS_D3,900*CLK_1M/3,50*CLK_1M);
	//eth_clk_set(ETH_CLKSRC_SYS_D3,get_cpu_clk()*2/3,50*CLK_1M);	
	eth_clk_set(ETH_CLKSRC_APLL_CLK,400*CLK_1M,50*CLK_1M);
	///GPIOD15-24 for 8626M;
	///GPIOD12	nRst;
	///GPIOD13    n_int;
	eth_set_pinmux(ETH_BANK2_GPIOD15_D23,ETH_CLK_OUT_GPIOD7_REG4_20,0);
	
	//power hold
	setbits_le32(P_PREG_AGPIO_O,(1<<8));
	clrbits_le32(P_PREG_AGPIO_EN_N,(1<<8));
	udelay(10);	//waiting reset end;
        aml_eth_init(bis);
        return 0;
}
#endif /* (CONFIG_CMD_NET) */

u32 get_board_rev(void)
{
    /*
    @todo implement this function
    */
	return 0x20;
}
static int  sdio_init(unsigned port)
{
	setbits_le32(P_PREG_CGPIO_EN_N,1<<5);

    return cpu_sdio_init(port);
}
static int  sdio_detect(unsigned port)
{
	return (readl(P_PREG_CGPIO_I)&(1<<5))?1:0;
}
static void sdio_pwr_prepare(unsigned port)
{
    /// @todo NOT FINISH
	///do nothing here
}
static void sdio_pwr_on(unsigned port)
{
	clrbits_le32(P_PREG_CGPIO_O,(1<<5));
	clrbits_le32(P_PREG_CGPIO_EN_N,(1<<5));//test_n
    /// @todo NOT FINISH
}
static void sdio_pwr_off(unsigned port)
{
	setbits_le32(P_PREG_CGPIO_O,(1<<5));
	clrbits_le32(P_PREG_CGPIO_EN_N,(1<<5));//test_n

	/// @todo NOT FINISH
}
static void board_mmc_register(unsigned port)
{
    AML_CARD_SD_INFO_t *aml_priv=cpu_sdio_get(port);
    if(aml_priv==NULL)
        return;
    struct mmc *mmc = (struct mmc *)malloc(sizeof(struct mmc));
    strncpy(mmc->name,aml_priv->name,31);
    mmc->priv = aml_priv;
	aml_priv->removed_flag = 1;
	aml_priv->inited_flag = 0;
	aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;
	mmc->send_cmd = aml_sd_send_cmd;
	mmc->set_ios = aml_sd_cfg_swth;
	mmc->init = aml_sd_init;
	mmc->rca = 1;
	mmc->voltages = MMC_VDD_33_34;
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS;
	//mmc->host_caps = MMC_MODE_4BIT;
	mmc->bus_width = 1;
	mmc->clock = 300000;
	mmc->f_min = 200000;
	mmc->f_max = 50000000;
	mmc_register(mmc);
}
int board_mmc_init(bd_t	*bis)
{
	board_mmc_register(SDIO_PORT_A);
//	board_mmc_register(SDIO_PORT_B);
//	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}

#ifdef CONFIG_SWITCH_BOOT_MODE

inline int get_key()
{
    int adc_val = get_adc_sample(4);
    printf("get_adc_sample(4): 0x%x\n", adc_val);
    return(((adc_val >= 0x240) && (adc_val < 0x320)) ? 1 : 0);
}

#if 0
void set_LCD_PWR_EN(unsigned    char    level)
{   
#if 0
	*((unsigned long*)0xc11080b0) &= ~(1<<13);	//PERIPHS_PIN_MUX_0	clr	bit[13]---disable TCON_CPV1

	*((unsigned long*)0xc11080b4) &= ~(1<<2);	//PERIPHS_PIN_MUX_1	clr	bit[2]---disable SD_D3_B

	*((unsigned long*)0xc11080d0) &= ~(1<<5);	//PERIPHS_PIN_MUX_8	clr	bit[5]---disable I2S_OUT[3]

	*((unsigned long*)0xc11080d4) &= ~(1<<18);	//PERIPHS_PIN_MUX_9	clr	bit[18]---disable WIFI Debug

	*((unsigned long*)0xc11080da) &= ~(1<<4);	//PERIPHS_PIN_MUX_11	clr	bit[4]---disable CP_POD

	*((unsigned long*)0xc11080e0) &= ~(1<<11);	//PERIPHS_PIN_MUX_12	clr	bit[11]---disable ENC_11


	*((unsigned long*)0xC1108030) &= ~(1<<7);	//GPIOA3		OEN = 0 (output) 

	if(level)
	{
		*((unsigned long*)0xC1108034) |= 1<<7;	//GPIOA3		Output level = 1 (pull high) 
	}
	else
	{
		*((unsigned long*)0xC1108034) &= ~(1<<7);	//GPIOA3		Output level = 0 (pull low) 
	}
#else
    //power unhold
    //printf("power hold\n");

	clear_mio_mux(8, 0x20);		
	clear_mio_mux(12, 0x800);		
	clear_mio_mux(0, 0x2000);		
	clear_mio_mux(1, 0x4);		
	clear_mio_mux(9, 0x40000);		
	clear_mio_mux(11, 0x10);		

	set_gpio_mode(GPIOA_bank_bit(3), GPIOA_bit_bit0_14(3), GPIO_OUTPUT_MODE);   // OEN = 0 (output) 
	set_gpio_val(GPIOA_bank_bit(3), GPIOA_bit_bit0_14(3), (level ? 1 : 0));   // Output level = 0 (pull high)
    //*(volatile unsigned long *)(0xC1108030) &= ~(1 << 12);              // OEN = 0 (output) 
    //*(volatile unsigned long *)(0xC1108034) &= ~(1 << 12);               // Output level = 0 (pull low)
#endif
}
#endif

#define is_ac_connected ((READ_CBUS_REG(ASSIST_HW_REV)&(1<<9))? 1:0)

int logo_display(void)
{
    run_command ("nand read ${loadaddr} ${p1start} ${p1size}", 0);
    run_command ("bmp display ${loadaddr} 0 0", 0);
    run_command ("lcd bl on", 0);

}
int switch_boot_mode(void)
{
    unsigned long hold_time = 50000, polling_time = 10000, tmp;
    unsigned long tag;

    tag = simple_strtoul (getenv ("tag"), NULL, 16);
    printf("tag = %d    !!!!!!!!!!!!!!!!!!\n", tag);

    if(tag == 3)
    {
        switch(get_kernel_cmd())
        {
            case RECOVERY_BOOT:
            {
                printf("RECOVERY_BOOT...\n");
                power_hold();
                logo_display();
                into_recovery();
                break;
            }
            case SYSTEM_REBOOT:
            {
                printf("SYSTEM_REBOOT...\n");
                power_hold();
                logo_display();
                return  1;
            }
            default:
            {
                printf("NORMAL_BOOT...\n");
                if(is_ac_connected)
                {
                    power_hold();
                }
                else
                {
                    powerkey_hold();
                }
                logo_display();
                break;
            }
        }
    }
    else
    {
        power_hold();
        printf("Upgrade step %d...\n", tag);
    }

    if(tag == 0)
    {
        if(upgrade_bootloader())
        {
            run_command ("set tag 1", 0);
            run_command ("save", 0);
            run_command ("reset", 0);
            hang();
        }
        else
        {
            printf("### ERROR:  u-boot write failed!!!\n");
            return  -1;
        }
    }
    else if((tag >0) && (tag < 3))
    {
        if(tag == 1)
        {
            if(upgrade_env())
            {
                run_command ("set tag 2", 0);
                run_command ("save", 0);
                run_command ("reset", 0);
                hang();
            }
            else
            {
                run_command ("defenv", 0);
                run_command ("save", 0);
            }
        }
        run_command ("set tag 3", 0);
        run_command ("save", 0);
        upgrade_partition();
        into_recovery();
    }

    saradc_enable();

    //added by Elvis for added fool idle
    get_key();
    get_key();
    
    while(hold_time > 0)
    {
        udelay(polling_time);
        tmp = get_key();
        printf("get_key(): %d\n", tmp);
        if(!tmp)  break;
        hold_time -= polling_time;
    }

    if(hold_time > 0)
    {
        printf("Normal Start...\n");
        return  1;
    }
    else
    {
        if(upgrade_bootloader())
        {
            run_command ("set tag 1", 0);
            run_command ("save", 0);
            run_command ("reset", 0);
            hang();
        }
        if(upgrade_env())
        {
            run_command ("set tag 2", 0);
            run_command ("save", 0);
            run_command ("reset", 0);
            hang();
        }
        run_command ("set tag 3", 0);
        run_command ("save", 0);
        upgrade_partition();
        into_recovery();
    }
}
#endif
