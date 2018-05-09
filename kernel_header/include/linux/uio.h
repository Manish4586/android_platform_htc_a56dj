#ifndef __LINUX_UIO_H
#define __LINUX_UIO_H


#include <linux/types.h>

/*
 *	Berkeley style UIO structures	-	Alan Cox 1994.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

struct iovec
{
	void *iov_base;	
	__kernel_size_t iov_len; 
};

 
#define UIO_FASTIOV	8
#define UIO_MAXIOV	1024


#endif
