
/*
 * Copyright (C) 2010 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _ARM_ENDIAN_H_
#define _ARM_ENDIAN_H_

#ifdef __GNUC__

#if !defined __ARM_ARCH_5__ && !defined __ARM_ARCH_5T__ && \
    !defined __ARM_ARCH_5TE__ && !defined __ARM_ARCH_5TEJ__ && \
    !defined __ARM_ARCH_4T__ && !defined __ARM_ARCH_4__

#define __swap16md(x) ({                                        \
    register u_int16_t _x = (x);                                \
    __asm volatile ("rev16 %0, %0" : "+l" (_x));                \
    _x;                                                         \
})

#define __swap32md(x) ({                                        \
    register u_int32_t _x = (x);                                \
    __asm volatile ("rev %0, %0" : "+l" (_x));                  \
    _x;                                                         \
})

#define __swap64md(x) ({                                        \
    u_int64_t _swap64md_x = (x);                                \
    (u_int64_t) __swap32md(_swap64md_x >> 32) |                 \
        (u_int64_t) __swap32md(_swap64md_x & 0xffffffff) << 32; \
})

#define MD_SWAP

#endif  
#endif  

#if defined(__ARMEB__)
#define _BYTE_ORDER _BIG_ENDIAN
#else
#define _BYTE_ORDER _LITTLE_ENDIAN
#endif
#define __STRICT_ALIGNMENT
#include <sys/types.h>
#include <sys/endian.h>

#endif  
