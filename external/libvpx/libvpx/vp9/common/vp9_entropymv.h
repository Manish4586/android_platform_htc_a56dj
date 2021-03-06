/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP9_COMMON_VP9_ENTROPYMV_H_
#define VP9_COMMON_VP9_ENTROPYMV_H_

#include "./vpx_config.h"

#include "vpx_dsp/prob.h"

#include "vp9/common/vp9_mv.h"

#ifdef __cplusplus
extern "C" {
#endif

struct VP9Common;

void vp9_init_mv_probs(struct VP9Common *cm);

void vp9_adapt_mv_probs(struct VP9Common *cm, int usehp);
int vp9_use_mv_hp(const MV *ref);

#define MV_UPDATE_PROB 252

#define MV_JOINTS     4
typedef enum {
  MV_JOINT_ZERO = 0,             
  MV_JOINT_HNZVZ = 1,            
  MV_JOINT_HZVNZ = 2,            
  MV_JOINT_HNZVNZ = 3,           
} MV_JOINT_TYPE;

static INLINE int mv_joint_vertical(MV_JOINT_TYPE type) {
  return type == MV_JOINT_HZVNZ || type == MV_JOINT_HNZVNZ;
}

static INLINE int mv_joint_horizontal(MV_JOINT_TYPE type) {
  return type == MV_JOINT_HNZVZ || type == MV_JOINT_HNZVNZ;
}

#define MV_CLASSES     11
typedef enum {
  MV_CLASS_0 = 0,      
  MV_CLASS_1 = 1,      
  MV_CLASS_2 = 2,      
  MV_CLASS_3 = 3,      
  MV_CLASS_4 = 4,      
  MV_CLASS_5 = 5,      
  MV_CLASS_6 = 6,      
  MV_CLASS_7 = 7,      
  MV_CLASS_8 = 8,      
  MV_CLASS_9 = 9,      
  MV_CLASS_10 = 10,    
} MV_CLASS_TYPE;

#define CLASS0_BITS    1  
#define CLASS0_SIZE    (1 << CLASS0_BITS)
#define MV_OFFSET_BITS (MV_CLASSES + CLASS0_BITS - 2)
#define MV_FP_SIZE 4

#define MV_MAX_BITS    (MV_CLASSES + CLASS0_BITS + 2)
#define MV_MAX         ((1 << MV_MAX_BITS) - 1)
#define MV_VALS        ((MV_MAX << 1) + 1)

#define MV_IN_USE_BITS 14
#define MV_UPP   ((1 << MV_IN_USE_BITS) - 1)
#define MV_LOW   (-(1 << MV_IN_USE_BITS))

extern const vpx_tree_index vp9_mv_joint_tree[];
extern const vpx_tree_index vp9_mv_class_tree[];
extern const vpx_tree_index vp9_mv_class0_tree[];
extern const vpx_tree_index vp9_mv_fp_tree[];

typedef struct {
  vpx_prob sign;
  vpx_prob classes[MV_CLASSES - 1];
  vpx_prob class0[CLASS0_SIZE - 1];
  vpx_prob bits[MV_OFFSET_BITS];
  vpx_prob class0_fp[CLASS0_SIZE][MV_FP_SIZE - 1];
  vpx_prob fp[MV_FP_SIZE - 1];
  vpx_prob class0_hp;
  vpx_prob hp;
} nmv_component;

typedef struct {
  vpx_prob joints[MV_JOINTS - 1];
  nmv_component comps[2];
} nmv_context;

static INLINE MV_JOINT_TYPE vp9_get_mv_joint(const MV *mv) {
  if (mv->row == 0) {
    return mv->col == 0 ? MV_JOINT_ZERO : MV_JOINT_HNZVZ;
  } else {
    return mv->col == 0 ? MV_JOINT_HZVNZ : MV_JOINT_HNZVNZ;
  }
}

MV_CLASS_TYPE vp9_get_mv_class(int z, int *offset);

typedef struct {
  unsigned int sign[2];
  unsigned int classes[MV_CLASSES];
  unsigned int class0[CLASS0_SIZE];
  unsigned int bits[MV_OFFSET_BITS][2];
  unsigned int class0_fp[CLASS0_SIZE][MV_FP_SIZE];
  unsigned int fp[MV_FP_SIZE];
  unsigned int class0_hp[2];
  unsigned int hp[2];
} nmv_component_counts;

typedef struct {
  unsigned int joints[MV_JOINTS];
  nmv_component_counts comps[2];
} nmv_context_counts;

void vp9_inc_mv(const MV *mv, nmv_context_counts *mvctx);

#ifdef __cplusplus
}  
#endif

#endif  
