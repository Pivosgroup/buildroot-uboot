
#include <common.h>
//#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <asm/arch/am_reg_addr.h>
#include <asm/arch/aml_card_sd.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/am_eth_reg.h>
#include <asm/arch/nand.h>

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
//	gd->bd->bi_arch_number=MACH_TYPE_MESON_6236M;
//	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;

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
#if 0    
    /*
    @todo implement this function
    */
    //eth_clk_set(ETH_CLKSRC_SYS_D3,900*CLK_1M/3,50*CLK_1M);
 	//eth_clk_set(ETH_CLKSRC_SYS_D3,get_cpu_clk()*2/3,50*CLK_1M);
	eth_clk_set(ETH_CLKSRC_APLL_CLK,400*CLK_1M,50*CLK_1M);
	/*for dpf_sz with ethernet*/	
    	eth_set_pinmux(ETH_BANK0_GPIOC3_C12,ETH_CLK_OUT_GPIOC12_REG3_1,0);
	CLEAR_CBUS_REG_MASK(PREG_ETHERNET_ADDR0, 1);
	SET_CBUS_REG_MASK(PREG_ETHERNET_ADDR0, (1 << 1));
	SET_CBUS_REG_MASK(PREG_ETHERNET_ADDR0, 1);
	udelay(100);
	/*reset*/
	CLEAR_CBUS_REG_MASK(PREG_FGPIO_EN_N,1<<0);
	CLEAR_CBUS_REG_MASK(PREG_FGPIO_O,1<<0);
	udelay(100);
	SET_CBUS_REG_MASK(PREG_FGPIO_O,1<<0);
	aml_eth_init(bis);
#endif
	return 0;
}
u32 get_board_rev(void)
{
    /*
    @todo implement this function
    */
	return 0x20;
}
static int  sdio_init(unsigned port)
{
//	setbits_le32(P_PREG_CGPIO_EN_N,1<<22);

    return cpu_sdio_init(port);
}
static int  sdio_detect(unsigned port)
{
//	return (readl(P_PREG_CGPIO_I)&(1<<22))?1:0;
    return 0;
}
static void sdio_pwr_prepare(unsigned port)
{
    /// @todo NOT FINISH
	///do nothing here
}
static void sdio_pwr_on(unsigned port)
{
	clrbits_le32(P_PREG_JTAG_GPIO_ADDR,(1<<20));
	clrbits_le32(P_PREG_JTAG_GPIO_ADDR,(1<<16));//test_n
    /// @todo NOT FINISH
}
static void sdio_pwr_off(unsigned port)
{
	setbits_le32(P_PREG_JTAG_GPIO_ADDR,(1<<20));
	clrbits_le32(P_PREG_JTAG_GPIO_ADDR,(1<<16));//test_n

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
	return 0;
}


void board_nand_pinmux( unsigned int en_dis )
{
//	printf("TODO SET A3 board pinmux");

	if(en_dis)
	{
		set_mio_mux(4,(0x1<<11) | (0xff<<14));
	}
	else
	{
		clear_mio_mux(4,(0x1<<11) | (0xff<<14));
	}
	
	return;	
}

/* every board have one config array */
//struct aml_platform_info aml_chips[]=
//{
//	[0]=
//	{	.mlc_mode=1,
//		.ran_mode=0,
//		.ecc_mode=NAND_ECC_BCH8_512,
//		.short_pgsz=0,
//		.rbpin_mode=1,
//		.timingset=7|(7<<5)|(0<<10),
//		.ce_sel=CE0
//	},
//#ifdef CONFIG_AMLROM_NANDBOOT
//   [1]=
//	{	.mlc_mode=1,
//		.ran_mode=0,
//		.ecc_mode=NAND_ECC_BCH60_1024 ,					//NOW SET BCH60 for simple , dont modify it , cause first pg romcode use BCH60
//		.short_pgsz=384,
//		.rbpin_mode=1,
//		.timingset=7|(7<<5)|(0<<10),
//		.ce_sel=CE0
//	},
//#endif
//};
struct aml_nand_platform aml_nand_mid_platform[] = {
    {
        .name = NAND_BOOT_NAME,
        .chip_enable_pad = AML_NAND_CE0,
        .ready_busy_pad = AML_NAND_CE0,
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_SHORT_MODE),
            },
        },
        .rbpin_mode=1,
        .short_pgsz=384,
        .ran_mode=0,
        .T_REA = 20,
        .T_RHOH = 15,
    },
    {
        .name = NAND_NORMAL_NAME,
        .chip_enable_pad = (AML_NAND_CE0) ,  //| (AML_NAND_CE1 << 4) | (AML_NAND_CE2 << 8) | (AML_NAND_CE3 << 12)),
        .ready_busy_pad = (AML_NAND_CE0) ,  //| (AML_NAND_CE0 << 4) | (AML_NAND_CE1 << 8) | (AML_NAND_CE1 << 12)),
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH30_MODE),
            },
        },
        .rbpin_mode = 1,
        .short_pgsz = 0,
        .ran_mode = 0,
        .T_REA = 20,
        .T_RHOH = 15,
    }
};

struct aml_nand_device aml_nand_mid_device = {
    .aml_nand_platform = aml_nand_mid_platform,
    .dev_num = ARRAY_SIZE(aml_nand_mid_platform),
};
#if defined(CONFIG_CMD_KGDB)

void
kgdb_serial_init(void)
{


}

void
putDebugChar (int c)
{
        serial_putc (c);
}

void
putDebugStr (const char *str)
{
        serial_puts (str);
}

int
getDebugChar (void)
{
        return serial_getc();
}

void
kgdb_interruptible (int yes)
{
        return;
}
int
kgdb_serial_tstc (void)
{
    return 0;
}
#endif

