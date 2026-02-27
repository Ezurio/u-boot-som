#ifndef _K3_COMMON_H_
#define _K3_COMMON_H_

#include <linux/types.h>

struct ddrss_patch {
    const u32 id;
	const struct ddr_patch_record *ctl_patch;
	const struct ddr_patch_record *pi_patch;
	const struct ddr_patch_record *phy_patch;
};

#endif /* _K3_COMMON_H_ */
