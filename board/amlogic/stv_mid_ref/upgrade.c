#include <linux/types.h>
#include <upgrade.h>
#include <common.h>
#include <command.h>

#define NAND_CLEAN

//simple_strtoul (argv[3], NULL, 16);
//partition
inline int size_align(int size)
{
    size = (((size / 0x10000) + 1) * 0x10000);
}

void self_check(void)
{
    unsigned long size;
    char	str[128];

    printf("u-boot come into the world!!!\n");
    size = simple_strtoul (getenv ("bootfilesize"), NULL, 16);
    run_command ("defenv", 0);
    sprintf(str, "set bootfilesize 0x%x", size);
    run_command (str, 0);
    run_command ("set tag 2", 0);
    run_command ("save", 0);
}

int upgrade_bootloader(void)
{
    int	i = 0, j = 0, retry = 2;
    char	str[128];
    unsigned long   size, size1;
    char    *filepath;
    
    printf("u-boot upgrading...\n");
    if(run_command ("mmcinfo", 0))
    {
        UPGRADE_DPRINT("##	ERROR: SD card not find!!!\n");
    }
    else
    {
        UPGRADE_DPRINT("Find SD card!!!\n");

        for(i = 0; i < SCAN_MMC_PARTITION; i++)
        {
            sprintf(str, "fatexist mmc 0:%d %s", (i + 1), UBOOTPATH);
            UPGRADE_DPRINT("command:    %s\n", str);
            if(!run_command (str, 0))
            {
                size = simple_strtoul (getenv ("bootfilesize"), NULL, 16);
                size1 = simple_strtoul (getenv ("filesize"), NULL, 16);
                //if(size != size1)
                {
                    UPGRADE_DPRINT("bootfilesize:%d   !=  filesize:%d\n", size, size1);
                    while(retry-- > 0)
                    {
                        sprintf(str, "fatload mmc 0:%d ${loadaddr} %s", (i + 1), UBOOTPATH);
                        UPGRADE_DPRINT("command:    %s\n", str);
                        run_command (str, 0);

                        run_command ("nand rom_protect off", 0);
#ifdef  NAND_CLEAN
                        int partition_num = simple_strtoul(getenv ("partnum"), NULL, 16);;
                        for(j = 0; j < partition_num; j++)
                        {
                            sprintf(str, "p%dpath", j);
                            filepath = getenv (str);
                            sprintf(str, "fatexist mmc 0:%d %s", (i + 1), filepath);
                            UPGRADE_DPRINT("command:    %s\n", str);
                            if(run_command (str, 0))
                            {
                                break;
                            }
                        }
                        if(j >= (partition_num - 1))
                        {
                            run_command ("nand scrub", 0);	
                        }
#endif				
                        run_command ("nand rom_write ${loadaddr} ${bootstart} ${bootsize}", 0);
                        run_command ("nand rom_protect on", 0);

                        size = simple_strtoul (getenv ("filesize"), NULL, 16);

                        sprintf(str, "set bootfilesize 0x%x", size);
                        run_command (str, 0);

                        UPGRADE_DPRINT("bootloader upgrade successful!\n");
                        run_command ("save", 0);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int upgrade_env(void)
{
    int	i = 0, partition_num = 0;
    char	str[128];
    char    *filepath;
    
    printf("environment upgrading...\n");
    if(run_command ("mmcinfo", 0))
    {
        UPGRADE_DPRINT("##	ERROR: SD card not find!!!\n");
    }
    else
    {
        UPGRADE_DPRINT("Find SD card!!!\n");
        
        for(i = 0; i < SCAN_MMC_PARTITION; i++)
        {
            sprintf(str, "fatexist mmc 0:%d ${envpath}", (i + 1));
            UPGRADE_DPRINT("command:    %s\n", str);
            if(!run_command (str, 0))
            {
                sprintf(str, "fatload mmc 0:%d ${loadaddr} ${envpath}", (i + 1));
                run_command (str, 0);
                run_command ("loadenv ${loadaddr}", 0);
                run_command ("save", 0);
                return  1;
            }
        }
    }
    return  0;
}

int upgrade_partition(void)
{
    int	i = 0, j = 0, ret = 0, partition_num = 0;
    char    upgrade_status_list[16];
    char	str[128];
    unsigned long   size, size1;
    char    *filepath;
    
    printf("partition upgrading...\n");
    if(run_command ("mmcinfo", 0))
    {
        UPGRADE_DPRINT("##	ERROR: SD card not find!!!\n");
    }
    else
    {
        UPGRADE_DPRINT("Find SD card!!!\n");
        memset(upgrade_status_list, 0, 16);
        partition_num = simple_strtoul(getenv ("partnum"), NULL, 16);

        for(i = 0; i < SCAN_MMC_PARTITION; i++)
        {
            for(j = 0; j < partition_num; j++)
            {
                if(!upgrade_status_list[j])
                {
                    sprintf(str, "p%dpath", j);
                    filepath = getenv (str);
                    sprintf(str, "fatexist mmc 0:%d %s", (i + 1), filepath);
                    UPGRADE_DPRINT("command:    %s\n", str);
                    if(!run_command (str, 0))
                    {
                        sprintf(str, "p%dfilesize", j);
                        size = simple_strtoul (getenv (str), NULL, 16);
                        size1 = simple_strtoul (getenv ("filesize"), NULL, 16);
                        //if(size != size1)
                        {
                            sprintf(str, "fatload mmc 0:%d ${loadaddr} ${p%dpath}", (i + 1), j);
                            UPGRADE_DPRINT("command:    %s\n", str);
                            run_command (str, 0);

                            sprintf(str, "nand erase ${p%dstart} ${p%dsize}", j, j);
                            run_command(str, 0);

                            sprintf(str, "nand write ${loadaddr} ${p%dstart} ${p%dsize}", j, j);
                            run_command (str, 0);
                            
                            sprintf(str, "set p%dfilesize ${filesize}", j);
                            run_command (str, 0);
                            upgrade_status_list[j] = 1;
                        }
                    }
                }
            }
        }
    }
    for(j = 0; j < partition_num; j++)
    {
        if(upgrade_status_list[j])
        {
            UPGRADE_DPRINT("p% upgrade successful!\n", j);
            ret = 1;
        }
    }
    if(ret)
    {
        run_command ("save", 0);
    }
    return ret;
}

inline void into_recovery(void)
{
    printf("Recovery Start...\n");
    run_command ("nand read 84100000 ${p1start} ${p1size}", 0);
    run_command ("nand read ${loadaddr} ${recoverystart} ${recoverysize}", 0);
    run_command ("bootm", 0);
}
