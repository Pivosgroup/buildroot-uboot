#include <config.h>
#include <asm/arch/romboot.h>
#include <asm/arch/firm/timing.h>
#include <asm/arch/firm/regs.h>
#include <asm/arch/firm/io.h>
#include <asm/arch/firm/nand.h>
#ifndef FIRMWARE_IN_ONE_FILE
#define STATIC_PREFIX
#else
#define STATIC_PREFIX static inline
#endif
#ifndef CONFIG_AML_UBOOT_MAGIC
#define CONFIG_AML_UBOOT_MAGIC 0x12345678
#endif
STATIC_PREFIX short check_sum(unsigned * addr,unsigned short check_sum,unsigned size)
{
    int i;
    unsigned short * p=(unsigned short *)addr;
    serial_put_dword(addr[15]);
    if(addr[15]!=CONFIG_AML_UBOOT_MAGIC)
        return -1;
#if 0
    for(i=0;i<size>>1;i++)
        check_sum^=p[i];
#endif
    return 0;
}

#ifdef CONFIG_BOARD_8726M_ARM
#define P_PREG_JTAG_GPIO_ADDR  (volatile unsigned long *)0xc110802c
#endif

STATIC_PREFIX int fw_load_intl(unsigned por_cfg,unsigned target,unsigned size)
{
    int rc=0, i;
    unsigned temp_addr;
#if CONFIG_UCL
#ifdef CONFIG_IMPROVE_UCL_DEC
    temp_addr=target;
#else
    temp_addr=target-0x800000;
#endif
#else
    temp_addr=target;
#endif

    unsigned * mem;
    unsigned len;
    switch(POR_GET_1ST_CFG(por_cfg))
    {
        case POR_1ST_NAND:
        case POR_1ST_NAND_RB:
            rc=nf_read(temp_addr,size);
            break;
        case POR_1ST_SPI :
        case POR_1ST_SPI_RESERVED :
            if(por_cfg&POR_ROM_BOOT_ENABLE)
                mem=(unsigned *)(NOR_START_ADDR+READ_SIZE);
            else
                mem=(unsigned *)(NOR_START_ADDR+READ_SIZE+ROM_SIZE);
            spi_init();
#if CONFIG_UCL==0
            if((rc=check_sum(target,0,0))!=0)
            {
                return rc;
            }
#endif
            serial_puts("Boot From SPI");
            memcpy((unsigned*)temp_addr,mem,size);
            break;
        case POR_1ST_SDIO_C:
            serial_puts("Boot From SDIO C\n");
            rc=sdio_read(temp_addr,size,POR_2ND_SDIO_C<<3);
            break;
        case POR_1ST_SDIO_B:
            rc=sdio_read(temp_addr,size,POR_2ND_SDIO_B<<3);break;
        case POR_1ST_SDIO_A:
            rc=sdio_read(temp_addr,size,POR_2ND_SDIO_A<<3);break;
           break;
        default:
           return 1;
    }
#if CONFIG_UCL
#ifndef CONFIG_IMPROVE_UCL_DEC
    if(rc==0)
	{
        serial_puts("ucl decompress\n");
		serial_put_dword(target);
#if UCL_DEBUG
		unsigned long *ucl_addr = (unsigned long *)target;
		for(i=0; i<1024*300/4; i++)
		{
			if((i%64) == 0)
			{
				serial_puts("\n");
				serial_put_dword(i*4);
			}
			serial_put_dword(ucl_addr[i]);
			serial_puts("	");
		}
#endif
        rc=uclDecompress(target,&len,temp_addr);
        serial_puts(rc?"decompress false\n":"decompress true\n");
		if(rc)
		{
			serial_puts("rc=");
			serial_put_dword(rc);
		}
    }
#endif //CONFIG_IMPROVE_UCL_DEC
#endif    
    if(rc==0)
        rc=check_sum((unsigned*)target,magic_info->crc[1],size);
    return rc;
}
STATIC_PREFIX int fw_init_extl(unsigned por_cfg)
{
	int rc=sdio_init(por_cfg);
	serial_put_dword(rc);
    serial_puts("sdio init\n");

    return rc;
}
STATIC_PREFIX int fw_load_extl(unsigned por_cfg,unsigned target,unsigned size)
{
    int ucl_dec_start,ucl_dec_end;
    unsigned temp_addr;
#if CONFIG_UCL
#ifdef CONFIG_IMPROVE_UCL_DEC
    temp_addr=target;
#else
    temp_addr=target-0x800000;
#endif
#else
    temp_addr=target;
#endif
    int rc=sdio_read(temp_addr,size,por_cfg);
    unsigned len;
    serial_put_dword(rc);
    serial_puts("sdio read\n");
#if CONFIG_UCL
#ifndef CONFIG_IMPROVE_UCL_DEC
    if(!rc){
	    serial_puts("ucl decompress\n");
	    //ucl_dec_start = TIMERE_GET();
	    rc=uclDecompress(target,&len,temp_addr);
        //ucl_dec_end = TIMERE_GET();
        serial_puts("decompress finished\n");
        //serial_puts("ucl dec time:");
        //serial_put_dword(ucl_dec_end-ucl_dec_start);
        //serial_puts("\n");
    }
#endif //CONFIG_IMPROVE_UCL_DEC
#endif

    if(!rc)
        rc=check_sum((unsigned*)target,magic_info->crc[1],size);
    return rc;
}
struct load_tbl_s{
    unsigned dest;
    unsigned src;
    unsigned size;
};
extern struct load_tbl_s __load_table[2];
STATIC_PREFIX void load_ext(unsigned por_cfg,unsigned bootid,unsigned target)
{
    int i,rc;
    unsigned temp_addr;
    unsigned len;
#if CONFIG_UCL
#ifdef CONFIG_IMPROVE_UCL_DEC
    temp_addr=target;
#else
    temp_addr=target-0x800000;
#endif
#else
    temp_addr=target;
#endif  
     
    if(bootid==0&&(POR_GET_1ST_CFG(por_cfg)==POR_1ST_SPI||POR_GET_1ST_CFG(por_cfg)==POR_1ST_SPI_RESERVED))
    {
        // spi boot
        if(por_cfg&POR_ROM_BOOT_ENABLE)
            temp_addr=(unsigned *)(NOR_START_ADDR+READ_SIZE);
        else
            temp_addr=(unsigned *)(NOR_START_ADDR+READ_SIZE+ROM_SIZE);
        
    }
        
    for(i=0;i<sizeof(__load_table)/sizeof(__load_table[0]);i++)
    {
        if(__load_table[i].size==0)
            continue;
#if CONFIG_UCL
#ifndef CONFIG_IMPROVE_UCL_DEC
        if( __load_table[i].size&(~0x3fffff))
        {
            rc=uclDecompress(__load_table[i].dest,&len,temp_addr+__load_table[i].src);
            if(rc)
            {
                serial_put_dword(i);
                serial_puts("decompress Fail\n");
            }
        }else
        
#endif //CONFIG_IMPROVE_UCL_DEC
#endif  
        memcpy(__load_table[i].dest,__load_table[i].src+temp_addr,__load_table[i].size&0x3fffff);      
    }
}

