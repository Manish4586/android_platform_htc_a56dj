/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "entropymv.h"

const MV_CONTEXT vp8_mv_update_probs[2] =
{
    {{
        237,
        246,
        253, 253, 254, 254, 254, 254, 254,
        254, 254, 254, 254, 254, 250, 250, 252, 254, 254
    }},
    {{
        231,
        243,
        245, 253, 254, 254, 254, 254, 254,
        254, 254, 254, 254, 254, 251, 251, 254, 254, 254
    }}
};
const MV_CONTEXT vp8_default_mv_context[2] =
{
    {{
        
        162,                                        
        128,                                        
        225, 146, 172, 147, 214,  39, 156,          
        128, 129, 132,  75, 145, 178, 206, 239, 254, 254 
    }},



    {{
        
        164,                                        
        128,
        204, 170, 119, 235, 140, 230, 228,
        128, 130, 130,  74, 148, 180, 203, 236, 254, 254 

    }}
};
