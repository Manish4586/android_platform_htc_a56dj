#ifndef _XT_CT_H
#define _XT_CT_H

#include <linux/types.h>

enum {
	XT_CT_NOTRACK		= 1 << 0,
	XT_CT_NOTRACK_ALIAS	= 1 << 1,
};

struct xt_ct_target_info {
	__u16 flags;
	__u16 zone;
	__u32 ct_events;
	__u32 exp_events;
	char helper[16];

	
	struct nf_conn	*ct __attribute__((aligned(8)));
};

struct xt_ct_target_info_v1 {
	__u16 flags;
	__u16 zone;
	__u32 ct_events;
	__u32 exp_events;
	char helper[16];
	char timeout[32];

	
	struct nf_conn	*ct __attribute__((aligned(8)));
};

#endif 
