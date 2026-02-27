#ifndef _IMX_COMMON_H_
#define _IMX_COMMON_H_

#include <linux/types.h>
#include <asm/arch/ddr.h>

void patch_ddr(struct dram_cfg_param *data, int data_size, 
	struct dram_cfg_param *patch, int patch_size);

int erase_ddr(struct dram_cfg_param *data, int data_size,
	u32 *regs_to_erase, int erase_size);

#endif /* _IMX_COMMON_H_ */
