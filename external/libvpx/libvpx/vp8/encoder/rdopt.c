/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include "vpx_config.h"
#include "vp8_rtcd.h"
#include "./vpx_dsp_rtcd.h"
#include "tokenize.h"
#include "treewriter.h"
#include "onyx_int.h"
#include "modecosts.h"
#include "encodeintra.h"
#include "pickinter.h"
#include "vp8/common/entropymode.h"
#include "vp8/common/reconinter.h"
#include "vp8/common/reconintra4x4.h"
#include "vp8/common/findnearmv.h"
#include "vp8/common/quant_common.h"
#include "encodemb.h"
#include "vp8/encoder/quantize.h"
#include "vpx_dsp/variance.h"
#include "mcomp.h"
#include "rdopt.h"
#include "vpx_mem/vpx_mem.h"
#include "vp8/common/systemdependent.h"
#if CONFIG_TEMPORAL_DENOISING
#include "denoising.h"
#endif
extern void vp8_update_zbin_extra(VP8_COMP *cpi, MACROBLOCK *x);

#define MAXF(a,b)            (((a) > (b)) ? (a) : (b))

typedef struct rate_distortion_struct
{
    int rate2;
    int rate_y;
    int rate_uv;
    int distortion2;
    int distortion_uv;
} RATE_DISTORTION;

typedef struct best_mode_struct
{
  int yrd;
  int rd;
  int intra_rd;
  MB_MODE_INFO mbmode;
  union b_mode_info bmodes[16];
  PARTITION_INFO partition;
} BEST_MODE;

static const int auto_speed_thresh[17] =
{
    1000,
    200,
    150,
    130,
    150,
    125,
    120,
    115,
    115,
    115,
    115,
    115,
    115,
    115,
    115,
    115,
    105
};

const MB_PREDICTION_MODE vp8_mode_order[MAX_MODES] =
{
    ZEROMV,
    DC_PRED,

    NEARESTMV,
    NEARMV,

    ZEROMV,
    NEARESTMV,

    ZEROMV,
    NEARESTMV,

    NEARMV,
    NEARMV,

    V_PRED,
    H_PRED,
    TM_PRED,

    NEWMV,
    NEWMV,
    NEWMV,

    SPLITMV,
    SPLITMV,
    SPLITMV,

    B_PRED,
};

const int vp8_ref_frame_order[MAX_MODES] =
{
    1,
    0,

    1,
    1,

    2,
    2,

    3,
    3,

    2,
    3,

    0,
    0,
    0,

    1,
    2,
    3,

    1,
    2,
    3,

    0,
};

static void fill_token_costs(
    int c[BLOCK_TYPES][COEF_BANDS][PREV_COEF_CONTEXTS][MAX_ENTROPY_TOKENS],
    const vp8_prob p[BLOCK_TYPES][COEF_BANDS][PREV_COEF_CONTEXTS][ENTROPY_NODES]
)
{
    int i, j, k;


    for (i = 0; i < BLOCK_TYPES; i++)
        for (j = 0; j < COEF_BANDS; j++)
            for (k = 0; k < PREV_COEF_CONTEXTS; k++)

                if (k == 0 && j > (i == 0))
                    vp8_cost_tokens2(c[i][j][k], p [i][j][k], vp8_coef_tree, 2);
                else
                    vp8_cost_tokens(c[i][j][k], p [i][j][k], vp8_coef_tree);
}

static const int rd_iifactor[32] =
{
    4, 4, 3, 2, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

static const int sad_per_bit16lut[QINDEX_RANGE] =
{
    2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,
    3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  3,  3,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  5,  5,  5,  5,  5,  5,
    5,  5,  5,  5,  5,  5,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,
    6,  6,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  9,  9,  9,  9,  9,  9,
    9,  9,  9,  9,  9,  9,  10, 10,
    10, 10, 10, 10, 10, 10, 11, 11,
    11, 11, 11, 11, 12, 12, 12, 12,
    12, 12, 13, 13, 13, 13, 14, 14
};
static const int sad_per_bit4lut[QINDEX_RANGE] =
{
    2,  2,  2,  2,  2,  2,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,
    3,  3,  3,  3,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  5,  5,
    5,  5,  5,  5,  6,  6,  6,  6,
    6,  6,  6,  6,  6,  6,  6,  6,
    7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  8,  8,  8,
    8,  8,  9,  9,  9,  9,  9,  9,
    10, 10, 10, 10, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 11, 11, 11,
    12, 12, 12, 12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 13, 13, 14,
    14, 14, 14, 14, 15, 15, 15, 15,
    16, 16, 16, 16, 17, 17, 17, 18,
    18, 18, 19, 19, 19, 20, 20, 20,
};

void vp8cx_initialize_me_consts(VP8_COMP *cpi, int QIndex)
{
    cpi->mb.sadperbit16 =  sad_per_bit16lut[QIndex];
    cpi->mb.sadperbit4  =  sad_per_bit4lut[QIndex];
}

void vp8_initialize_rd_consts(VP8_COMP *cpi, MACROBLOCK *x, int Qvalue)
{
    int q;
    int i;
    double capped_q = (Qvalue < 160) ? (double)Qvalue : 160.0;
    double rdconst = 2.80;

    vp8_clear_system_state();

    cpi->RDMULT = (int)(rdconst * (capped_q * capped_q));

    
    if (cpi->mb.zbin_over_quant  > 0)
    {
        double oq_factor;
        double modq;

        oq_factor = 1.0 + ((double)0.0015625 * cpi->mb.zbin_over_quant);
        modq = (int)((double)capped_q * oq_factor);
        cpi->RDMULT = (int)(rdconst * (modq * modq));
    }

    if (cpi->pass == 2 && (cpi->common.frame_type != KEY_FRAME))
    {
        if (cpi->twopass.next_iiratio > 31)
            cpi->RDMULT += (cpi->RDMULT * rd_iifactor[31]) >> 4;
        else
            cpi->RDMULT +=
                (cpi->RDMULT * rd_iifactor[cpi->twopass.next_iiratio]) >> 4;
    }

    cpi->mb.errorperbit = (cpi->RDMULT / 110);
    cpi->mb.errorperbit += (cpi->mb.errorperbit==0);

    vp8_set_speed_features(cpi);

    for (i = 0; i < MAX_MODES; i++)
    {
        x->mode_test_hit_counts[i] = 0;
    }

    q = (int)pow(Qvalue, 1.25);

    if (q < 8)
        q = 8;

    if (cpi->RDMULT > 1000)
    {
        cpi->RDDIV = 1;
        cpi->RDMULT /= 100;

        for (i = 0; i < MAX_MODES; i++)
        {
            if (cpi->sf.thresh_mult[i] < INT_MAX)
            {
                x->rd_threshes[i] = cpi->sf.thresh_mult[i] * q / 100;
            }
            else
            {
                x->rd_threshes[i] = INT_MAX;
            }

            cpi->rd_baseline_thresh[i] = x->rd_threshes[i];
        }
    }
    else
    {
        cpi->RDDIV = 100;

        for (i = 0; i < MAX_MODES; i++)
        {
            if (cpi->sf.thresh_mult[i] < (INT_MAX / q))
            {
                x->rd_threshes[i] = cpi->sf.thresh_mult[i] * q;
            }
            else
            {
                x->rd_threshes[i] = INT_MAX;
            }

            cpi->rd_baseline_thresh[i] = x->rd_threshes[i];
        }
    }

    {
      
      FRAME_CONTEXT *l = &cpi->lfc_n;

      if(cpi->common.refresh_alt_ref_frame)
          l = &cpi->lfc_a;
      else if(cpi->common.refresh_golden_frame)
          l = &cpi->lfc_g;

      fill_token_costs(
          cpi->mb.token_costs,
          (const vp8_prob( *)[8][3][11]) l->coef_probs
      );


      
      vp8_init_mode_costs(cpi);
    }

}

void vp8_auto_select_speed(VP8_COMP *cpi)
{
    int milliseconds_for_compress = (int)(1000000 / cpi->framerate);

    milliseconds_for_compress = milliseconds_for_compress * (16 - cpi->oxcf.cpu_used) / 16;

#if 0

    if (0)
    {
        FILE *f;

        f = fopen("speed.stt", "a");
        fprintf(f, " %8ld %10ld %10ld %10ld\n",
                cpi->common.current_video_frame, cpi->Speed, milliseconds_for_compress, cpi->avg_pick_mode_time);
        fclose(f);
    }

#endif

    if (cpi->avg_pick_mode_time < milliseconds_for_compress && (cpi->avg_encode_time - cpi->avg_pick_mode_time) < milliseconds_for_compress)
    {
        if (cpi->avg_pick_mode_time == 0)
        {
            cpi->Speed = 4;
        }
        else
        {
            if (milliseconds_for_compress * 100 < cpi->avg_encode_time * 95)
            {
                cpi->Speed          += 2;
                cpi->avg_pick_mode_time = 0;
                cpi->avg_encode_time = 0;

                if (cpi->Speed > 16)
                {
                    cpi->Speed = 16;
                }
            }

            if (milliseconds_for_compress * 100 > cpi->avg_encode_time * auto_speed_thresh[cpi->Speed])
            {
                cpi->Speed          -= 1;
                cpi->avg_pick_mode_time = 0;
                cpi->avg_encode_time = 0;

                
                if (cpi->Speed < 4)
                {
                    cpi->Speed = 4;
                }
            }
        }
    }
    else
    {
        cpi->Speed += 4;

        if (cpi->Speed > 16)
            cpi->Speed = 16;


        cpi->avg_pick_mode_time = 0;
        cpi->avg_encode_time = 0;
    }
}

int vp8_block_error_c(short *coeff, short *dqcoeff)
{
    int i;
    int error = 0;

    for (i = 0; i < 16; i++)
    {
        int this_diff = coeff[i] - dqcoeff[i];
        error += this_diff * this_diff;
    }

    return error;
}

int vp8_mbblock_error_c(MACROBLOCK *mb, int dc)
{
    BLOCK  *be;
    BLOCKD *bd;
    int i, j;
    int berror, error = 0;

    for (i = 0; i < 16; i++)
    {
        be = &mb->block[i];
        bd = &mb->e_mbd.block[i];

        berror = 0;

        for (j = dc; j < 16; j++)
        {
            int this_diff = be->coeff[j] - bd->dqcoeff[j];
            berror += this_diff * this_diff;
        }

        error += berror;
    }

    return error;
}

int vp8_mbuverror_c(MACROBLOCK *mb)
{

    BLOCK  *be;
    BLOCKD *bd;


    int i;
    int error = 0;

    for (i = 16; i < 24; i++)
    {
        be = &mb->block[i];
        bd = &mb->e_mbd.block[i];

        error += vp8_block_error_c(be->coeff, bd->dqcoeff);
    }

    return error;
}

int VP8_UVSSE(MACROBLOCK *x)
{
    unsigned char *uptr, *vptr;
    unsigned char *upred_ptr = (*(x->block[16].base_src) + x->block[16].src);
    unsigned char *vpred_ptr = (*(x->block[20].base_src) + x->block[20].src);
    int uv_stride = x->block[16].src_stride;

    unsigned int sse1 = 0;
    unsigned int sse2 = 0;
    int mv_row = x->e_mbd.mode_info_context->mbmi.mv.as_mv.row;
    int mv_col = x->e_mbd.mode_info_context->mbmi.mv.as_mv.col;
    int offset;
    int pre_stride = x->e_mbd.pre.uv_stride;

    if (mv_row < 0)
        mv_row -= 1;
    else
        mv_row += 1;

    if (mv_col < 0)
        mv_col -= 1;
    else
        mv_col += 1;

    mv_row /= 2;
    mv_col /= 2;

    offset = (mv_row >> 3) * pre_stride + (mv_col >> 3);
    uptr = x->e_mbd.pre.u_buffer + offset;
    vptr = x->e_mbd.pre.v_buffer + offset;

    if ((mv_row | mv_col) & 7)
    {
        vpx_sub_pixel_variance8x8(uptr, pre_stride,
            mv_col & 7, mv_row & 7, upred_ptr, uv_stride, &sse2);
        vpx_sub_pixel_variance8x8(vptr, pre_stride,
            mv_col & 7, mv_row & 7, vpred_ptr, uv_stride, &sse1);
        sse2 += sse1;
    }
    else
    {
        vpx_variance8x8(uptr, pre_stride,
            upred_ptr, uv_stride, &sse2);
        vpx_variance8x8(vptr, pre_stride,
            vpred_ptr, uv_stride, &sse1);
        sse2 += sse1;
    }
    return sse2;

}

static int cost_coeffs(MACROBLOCK *mb, BLOCKD *b, int type, ENTROPY_CONTEXT *a, ENTROPY_CONTEXT *l)
{
    int c = !type;              
    int eob = (int)(*b->eob);
    int pt ;    
    int cost = 0;
    short *qcoeff_ptr = b->qcoeff;

    VP8_COMBINEENTROPYCONTEXTS(pt, *a, *l);

    assert(eob <= 16);
    for (; c < eob; c++)
    {
        const int v = qcoeff_ptr[vp8_default_zig_zag1d[c]];
        const int t = vp8_dct_value_tokens_ptr[v].Token;
        cost += mb->token_costs [type] [vp8_coef_bands[c]] [pt] [t];
        cost += vp8_dct_value_cost_ptr[v];
        pt = vp8_prev_token_class[t];
    }

    if (c < 16)
        cost += mb->token_costs [type] [vp8_coef_bands[c]] [pt] [DCT_EOB_TOKEN];

    pt = (c != !type); 
    *a = *l = pt;

    return cost;
}

static int vp8_rdcost_mby(MACROBLOCK *mb)
{
    int cost = 0;
    int b;
    MACROBLOCKD *x = &mb->e_mbd;
    ENTROPY_CONTEXT_PLANES t_above, t_left;
    ENTROPY_CONTEXT *ta;
    ENTROPY_CONTEXT *tl;

    memcpy(&t_above, mb->e_mbd.above_context, sizeof(ENTROPY_CONTEXT_PLANES));
    memcpy(&t_left, mb->e_mbd.left_context, sizeof(ENTROPY_CONTEXT_PLANES));

    ta = (ENTROPY_CONTEXT *)&t_above;
    tl = (ENTROPY_CONTEXT *)&t_left;

    for (b = 0; b < 16; b++)
        cost += cost_coeffs(mb, x->block + b, PLANE_TYPE_Y_NO_DC,
                    ta + vp8_block2above[b], tl + vp8_block2left[b]);

    cost += cost_coeffs(mb, x->block + 24, PLANE_TYPE_Y2,
                ta + vp8_block2above[24], tl + vp8_block2left[24]);

    return cost;
}

static void macro_block_yrd( MACROBLOCK *mb,
                             int *Rate,
                             int *Distortion)
{
    int b;
    MACROBLOCKD *const x = &mb->e_mbd;
    BLOCK   *const mb_y2 = mb->block + 24;
    BLOCKD *const x_y2  = x->block + 24;
    short *Y2DCPtr = mb_y2->src_diff;
    BLOCK *beptr;
    int d;

    vp8_subtract_mby( mb->src_diff, *(mb->block[0].base_src),
        mb->block[0].src_stride,  mb->e_mbd.predictor, 16);

    
    for (beptr = mb->block; beptr < mb->block + 16; beptr += 2)
    {
        mb->short_fdct8x4(beptr->src_diff, beptr->coeff, 32);
        *Y2DCPtr++ = beptr->coeff[0];
        *Y2DCPtr++ = beptr->coeff[16];
    }

    
    mb->short_walsh4x4(mb_y2->src_diff, mb_y2->coeff, 8);

    
    for (b = 0; b < 16; b++)
    {
        mb->quantize_b(&mb->block[b], &mb->e_mbd.block[b]);
    }

    
    mb->quantize_b(mb_y2, x_y2);

    
    d = vp8_mbblock_error(mb, 1) << 2;
    d += vp8_block_error(mb_y2->coeff, x_y2->dqcoeff);

    *Distortion = (d >> 4);

    
    *Rate = vp8_rdcost_mby(mb);
}

static void copy_predictor(unsigned char *dst, const unsigned char *predictor)
{
    const unsigned int *p = (const unsigned int *)predictor;
    unsigned int *d = (unsigned int *)dst;
    d[0] = p[0];
    d[4] = p[4];
    d[8] = p[8];
    d[12] = p[12];
}
static int rd_pick_intra4x4block(
    MACROBLOCK *x,
    BLOCK *be,
    BLOCKD *b,
    B_PREDICTION_MODE *best_mode,
    const int *bmode_costs,
    ENTROPY_CONTEXT *a,
    ENTROPY_CONTEXT *l,

    int *bestrate,
    int *bestratey,
    int *bestdistortion)
{
    B_PREDICTION_MODE mode;
    int best_rd = INT_MAX;
    int rate = 0;
    int distortion;

    ENTROPY_CONTEXT ta = *a, tempa = *a;
    ENTROPY_CONTEXT tl = *l, templ = *l;
    DECLARE_ALIGNED(16, unsigned char,  best_predictor[16*4]);
    DECLARE_ALIGNED(16, short, best_dqcoeff[16]);
    int dst_stride = x->e_mbd.dst.y_stride;
    unsigned char *dst = x->e_mbd.dst.y_buffer + b->offset;

    unsigned char *Above = dst - dst_stride;
    unsigned char *yleft = dst - 1;
    unsigned char top_left = Above[-1];

    for (mode = B_DC_PRED; mode <= B_HU_PRED; mode++)
    {
        int this_rd;
        int ratey;

        rate = bmode_costs[mode];

        vp8_intra4x4_predict(Above, yleft, dst_stride, mode,
                             b->predictor, 16, top_left);
        vp8_subtract_b(be, b, 16);
        x->short_fdct4x4(be->src_diff, be->coeff, 32);
        x->quantize_b(be, b);

        tempa = ta;
        templ = tl;

        ratey = cost_coeffs(x, b, PLANE_TYPE_Y_WITH_DC, &tempa, &templ);
        rate += ratey;
        distortion = vp8_block_error(be->coeff, b->dqcoeff) >> 2;

        this_rd = RDCOST(x->rdmult, x->rddiv, rate, distortion);

        if (this_rd < best_rd)
        {
            *bestrate = rate;
            *bestratey = ratey;
            *bestdistortion = distortion;
            best_rd = this_rd;
            *best_mode = mode;
            *a = tempa;
            *l = templ;
            copy_predictor(best_predictor, b->predictor);
            memcpy(best_dqcoeff, b->dqcoeff, 32);
        }
    }
    b->bmi.as_mode = *best_mode;

    vp8_short_idct4x4llm(best_dqcoeff, best_predictor, 16, dst, dst_stride);

    return best_rd;
}

static int rd_pick_intra4x4mby_modes(MACROBLOCK *mb, int *Rate,
                                     int *rate_y, int *Distortion, int best_rd)
{
    MACROBLOCKD *const xd = &mb->e_mbd;
    int i;
    int cost = mb->mbmode_cost [xd->frame_type] [B_PRED];
    int distortion = 0;
    int tot_rate_y = 0;
    int64_t total_rd = 0;
    ENTROPY_CONTEXT_PLANES t_above, t_left;
    ENTROPY_CONTEXT *ta;
    ENTROPY_CONTEXT *tl;
    const int *bmode_costs;

    memcpy(&t_above, mb->e_mbd.above_context, sizeof(ENTROPY_CONTEXT_PLANES));
    memcpy(&t_left, mb->e_mbd.left_context, sizeof(ENTROPY_CONTEXT_PLANES));

    ta = (ENTROPY_CONTEXT *)&t_above;
    tl = (ENTROPY_CONTEXT *)&t_left;

    intra_prediction_down_copy(xd, xd->dst.y_buffer - xd->dst.y_stride + 16);

    bmode_costs = mb->inter_bmode_costs;

    for (i = 0; i < 16; i++)
    {
        MODE_INFO *const mic = xd->mode_info_context;
        const int mis = xd->mode_info_stride;
        B_PREDICTION_MODE UNINITIALIZED_IS_SAFE(best_mode);
        int UNINITIALIZED_IS_SAFE(r), UNINITIALIZED_IS_SAFE(ry), UNINITIALIZED_IS_SAFE(d);

        if (mb->e_mbd.frame_type == KEY_FRAME)
        {
            const B_PREDICTION_MODE A = above_block_mode(mic, i, mis);
            const B_PREDICTION_MODE L = left_block_mode(mic, i);

            bmode_costs  = mb->bmode_costs[A][L];
        }

        total_rd += rd_pick_intra4x4block(
            mb, mb->block + i, xd->block + i, &best_mode, bmode_costs,
            ta + vp8_block2above[i],
            tl + vp8_block2left[i], &r, &ry, &d);

        cost += r;
        distortion += d;
        tot_rate_y += ry;

        mic->bmi[i].as_mode = best_mode;

        if(total_rd >= (int64_t)best_rd)
            break;
    }

    if(total_rd >= (int64_t)best_rd)
        return INT_MAX;

    *Rate = cost;
    *rate_y = tot_rate_y;
    *Distortion = distortion;

    return RDCOST(mb->rdmult, mb->rddiv, cost, distortion);
}


static int rd_pick_intra16x16mby_mode(MACROBLOCK *x,
                                      int *Rate,
                                      int *rate_y,
                                      int *Distortion)
{
    MB_PREDICTION_MODE mode;
    MB_PREDICTION_MODE UNINITIALIZED_IS_SAFE(mode_selected);
    int rate, ratey;
    int distortion;
    int best_rd = INT_MAX;
    int this_rd;
    MACROBLOCKD *xd = &x->e_mbd;

    
    for (mode = DC_PRED; mode <= TM_PRED; mode++)
    {
        xd->mode_info_context->mbmi.mode = mode;

        vp8_build_intra_predictors_mby_s(xd,
                                         xd->dst.y_buffer - xd->dst.y_stride,
                                         xd->dst.y_buffer - 1,
                                         xd->dst.y_stride,
                                         xd->predictor,
                                         16);

        macro_block_yrd(x, &ratey, &distortion);
        rate = ratey + x->mbmode_cost[xd->frame_type]
                                     [xd->mode_info_context->mbmi.mode];

        this_rd = RDCOST(x->rdmult, x->rddiv, rate, distortion);

        if (this_rd < best_rd)
        {
            mode_selected = mode;
            best_rd = this_rd;
            *Rate = rate;
            *rate_y = ratey;
            *Distortion = distortion;
        }
    }

    xd->mode_info_context->mbmi.mode = mode_selected;
    return best_rd;
}

static int rd_cost_mbuv(MACROBLOCK *mb)
{
    int b;
    int cost = 0;
    MACROBLOCKD *x = &mb->e_mbd;
    ENTROPY_CONTEXT_PLANES t_above, t_left;
    ENTROPY_CONTEXT *ta;
    ENTROPY_CONTEXT *tl;

    memcpy(&t_above, mb->e_mbd.above_context, sizeof(ENTROPY_CONTEXT_PLANES));
    memcpy(&t_left, mb->e_mbd.left_context, sizeof(ENTROPY_CONTEXT_PLANES));

    ta = (ENTROPY_CONTEXT *)&t_above;
    tl = (ENTROPY_CONTEXT *)&t_left;

    for (b = 16; b < 24; b++)
        cost += cost_coeffs(mb, x->block + b, PLANE_TYPE_UV,
                    ta + vp8_block2above[b], tl + vp8_block2left[b]);

    return cost;
}


static int rd_inter16x16_uv(VP8_COMP *cpi, MACROBLOCK *x, int *rate,
                            int *distortion, int fullpixel)
{
    (void)cpi;
    (void)fullpixel;

    vp8_build_inter16x16_predictors_mbuv(&x->e_mbd);
    vp8_subtract_mbuv(x->src_diff,
        x->src.u_buffer, x->src.v_buffer, x->src.uv_stride,
        &x->e_mbd.predictor[256], &x->e_mbd.predictor[320], 8);

    vp8_transform_mbuv(x);
    vp8_quantize_mbuv(x);

    *rate       = rd_cost_mbuv(x);
    *distortion = vp8_mbuverror(x) / 4;

    return RDCOST(x->rdmult, x->rddiv, *rate, *distortion);
}

static int rd_inter4x4_uv(VP8_COMP *cpi, MACROBLOCK *x, int *rate,
                          int *distortion, int fullpixel)
{
    (void)cpi;
    (void)fullpixel;

    vp8_build_inter4x4_predictors_mbuv(&x->e_mbd);
    vp8_subtract_mbuv(x->src_diff,
        x->src.u_buffer, x->src.v_buffer, x->src.uv_stride,
        &x->e_mbd.predictor[256], &x->e_mbd.predictor[320], 8);

    vp8_transform_mbuv(x);
    vp8_quantize_mbuv(x);

    *rate       = rd_cost_mbuv(x);
    *distortion = vp8_mbuverror(x) / 4;

    return RDCOST(x->rdmult, x->rddiv, *rate, *distortion);
}

static void rd_pick_intra_mbuv_mode(MACROBLOCK *x, int *rate,
                                    int *rate_tokenonly, int *distortion)
{
    MB_PREDICTION_MODE mode;
    MB_PREDICTION_MODE UNINITIALIZED_IS_SAFE(mode_selected);
    int best_rd = INT_MAX;
    int UNINITIALIZED_IS_SAFE(d), UNINITIALIZED_IS_SAFE(r);
    int rate_to;
    MACROBLOCKD *xd = &x->e_mbd;

    for (mode = DC_PRED; mode <= TM_PRED; mode++)
    {
        int this_rate;
        int this_distortion;
        int this_rd;

        xd->mode_info_context->mbmi.uv_mode = mode;

        vp8_build_intra_predictors_mbuv_s(xd,
                                          xd->dst.u_buffer - xd->dst.uv_stride,
                                          xd->dst.v_buffer - xd->dst.uv_stride,
                                          xd->dst.u_buffer - 1,
                                          xd->dst.v_buffer - 1,
                                          xd->dst.uv_stride,
                                          &xd->predictor[256], &xd->predictor[320],
                                          8);


        vp8_subtract_mbuv(x->src_diff,
                      x->src.u_buffer, x->src.v_buffer, x->src.uv_stride,
                      &xd->predictor[256], &xd->predictor[320], 8);
        vp8_transform_mbuv(x);
        vp8_quantize_mbuv(x);

        rate_to = rd_cost_mbuv(x);
        this_rate = rate_to + x->intra_uv_mode_cost[xd->frame_type][xd->mode_info_context->mbmi.uv_mode];

        this_distortion = vp8_mbuverror(x) / 4;

        this_rd = RDCOST(x->rdmult, x->rddiv, this_rate, this_distortion);

        if (this_rd < best_rd)
        {
            best_rd = this_rd;
            d = this_distortion;
            r = this_rate;
            *rate_tokenonly = rate_to;
            mode_selected = mode;
        }
    }

    *rate = r;
    *distortion = d;

    xd->mode_info_context->mbmi.uv_mode = mode_selected;
}

int vp8_cost_mv_ref(MB_PREDICTION_MODE m, const int near_mv_ref_ct[4])
{
    vp8_prob p [VP8_MVREFS-1];
    assert(NEARESTMV <= m  &&  m <= SPLITMV);
    vp8_mv_ref_probs(p, near_mv_ref_ct);
    return vp8_cost_token(vp8_mv_ref_tree, p,
                          vp8_mv_ref_encoding_array + (m - NEARESTMV));
}

void vp8_set_mbmode_and_mvs(MACROBLOCK *x, MB_PREDICTION_MODE mb, int_mv *mv)
{
    x->e_mbd.mode_info_context->mbmi.mode = mb;
    x->e_mbd.mode_info_context->mbmi.mv.as_int = mv->as_int;
}

static int labels2mode(
    MACROBLOCK *x,
    int const *labelings, int which_label,
    B_PREDICTION_MODE this_mode,
    int_mv *this_mv, int_mv *best_ref_mv,
    int *mvcost[2]
)
{
    MACROBLOCKD *const xd = & x->e_mbd;
    MODE_INFO *const mic = xd->mode_info_context;
    const int mis = xd->mode_info_stride;

    int cost = 0;
    int thismvcost = 0;


    int i = 0;

    do
    {
        BLOCKD *const d = xd->block + i;
        const int row = i >> 2,  col = i & 3;

        B_PREDICTION_MODE m;

        if (labelings[i] != which_label)
            continue;

        if (col  &&  labelings[i] == labelings[i-1])
            m = LEFT4X4;
        else if (row  &&  labelings[i] == labelings[i-4])
            m = ABOVE4X4;
        else
        {
            switch (m = this_mode)
            {
            case NEW4X4 :
                thismvcost  = vp8_mv_bit_cost(this_mv, best_ref_mv, mvcost, 102);
                break;
            case LEFT4X4:
                this_mv->as_int = col ? d[-1].bmi.mv.as_int : left_block_mv(mic, i);
                break;
            case ABOVE4X4:
                this_mv->as_int = row ? d[-4].bmi.mv.as_int : above_block_mv(mic, i, mis);
                break;
            case ZERO4X4:
                this_mv->as_int = 0;
                break;
            default:
                break;
            }

            if (m == ABOVE4X4)  
            {
                int_mv left_mv;

                left_mv.as_int = col ? d[-1].bmi.mv.as_int :
                                        left_block_mv(mic, i);

                if (left_mv.as_int == this_mv->as_int)
                    m = LEFT4X4;
            }

            cost = x->inter_bmode_costs[ m];
        }

        d->bmi.mv.as_int = this_mv->as_int;

        x->partition_info->bmi[i].mode = m;
        x->partition_info->bmi[i].mv.as_int = this_mv->as_int;

    }
    while (++i < 16);

    cost += thismvcost ;
    return cost;
}

static int rdcost_mbsegment_y(MACROBLOCK *mb, const int *labels,
                              int which_label, ENTROPY_CONTEXT *ta,
                              ENTROPY_CONTEXT *tl)
{
    int cost = 0;
    int b;
    MACROBLOCKD *x = &mb->e_mbd;

    for (b = 0; b < 16; b++)
        if (labels[ b] == which_label)
            cost += cost_coeffs(mb, x->block + b, PLANE_TYPE_Y_WITH_DC,
                                ta + vp8_block2above[b],
                                tl + vp8_block2left[b]);

    return cost;

}
static unsigned int vp8_encode_inter_mb_segment(MACROBLOCK *x, int const *labels, int which_label)
{
    int i;
    unsigned int distortion = 0;
    int pre_stride = x->e_mbd.pre.y_stride;
    unsigned char *base_pre = x->e_mbd.pre.y_buffer;


    for (i = 0; i < 16; i++)
    {
        if (labels[i] == which_label)
        {
            BLOCKD *bd = &x->e_mbd.block[i];
            BLOCK *be = &x->block[i];

            vp8_build_inter_predictors_b(bd, 16, base_pre, pre_stride, x->e_mbd.subpixel_predict);
            vp8_subtract_b(be, bd, 16);
            x->short_fdct4x4(be->src_diff, be->coeff, 32);
            x->quantize_b(be, bd);

            distortion += vp8_block_error(be->coeff, bd->dqcoeff);
        }
    }

    return distortion;
}


static const unsigned int segmentation_to_sseshift[4] = {3, 3, 2, 0};


typedef struct
{
  int_mv *ref_mv;
  int_mv mvp;

  int segment_rd;
  int segment_num;
  int r;
  int d;
  int segment_yrate;
  B_PREDICTION_MODE modes[16];
  int_mv mvs[16];
  unsigned char eobs[16];

  int mvthresh;
  int *mdcounts;

  int_mv sv_mvp[4]; 
  int sv_istep[2];  

} BEST_SEG_INFO;


static void rd_check_segment(VP8_COMP *cpi, MACROBLOCK *x,
                             BEST_SEG_INFO *bsi, unsigned int segmentation)
{
    int i;
    int const *labels;
    int br = 0;
    int bd = 0;
    B_PREDICTION_MODE this_mode;


    int label_count;
    int this_segment_rd = 0;
    int label_mv_thresh;
    int rate = 0;
    int sbr = 0;
    int sbd = 0;
    int segmentyrate = 0;

    vp8_variance_fn_ptr_t *v_fn_ptr;

    ENTROPY_CONTEXT_PLANES t_above, t_left;
    ENTROPY_CONTEXT *ta;
    ENTROPY_CONTEXT *tl;
    ENTROPY_CONTEXT_PLANES t_above_b, t_left_b;
    ENTROPY_CONTEXT *ta_b;
    ENTROPY_CONTEXT *tl_b;

    memcpy(&t_above, x->e_mbd.above_context, sizeof(ENTROPY_CONTEXT_PLANES));
    memcpy(&t_left, x->e_mbd.left_context, sizeof(ENTROPY_CONTEXT_PLANES));

    ta = (ENTROPY_CONTEXT *)&t_above;
    tl = (ENTROPY_CONTEXT *)&t_left;
    ta_b = (ENTROPY_CONTEXT *)&t_above_b;
    tl_b = (ENTROPY_CONTEXT *)&t_left_b;

    br = 0;
    bd = 0;

    v_fn_ptr = &cpi->fn_ptr[segmentation];
    labels = vp8_mbsplits[segmentation];
    label_count = vp8_mbsplit_count[segmentation];

    label_mv_thresh = 1 * bsi->mvthresh / label_count ;

    
    rate = vp8_cost_token(vp8_mbsplit_tree, vp8_mbsplit_probs, vp8_mbsplit_encodings + segmentation);
    rate += vp8_cost_mv_ref(SPLITMV, bsi->mdcounts);
    this_segment_rd += RDCOST(x->rdmult, x->rddiv, rate, 0);
    br += rate;

    for (i = 0; i < label_count; i++)
    {
        int_mv mode_mv[B_MODE_COUNT];
        int best_label_rd = INT_MAX;
        B_PREDICTION_MODE mode_selected = ZERO4X4;
        int bestlabelyrate = 0;

        
        for (this_mode = LEFT4X4; this_mode <= NEW4X4 ; this_mode ++)
        {
            int this_rd;
            int distortion;
            int labelyrate;
            ENTROPY_CONTEXT_PLANES t_above_s, t_left_s;
            ENTROPY_CONTEXT *ta_s;
            ENTROPY_CONTEXT *tl_s;

            memcpy(&t_above_s, &t_above, sizeof(ENTROPY_CONTEXT_PLANES));
            memcpy(&t_left_s, &t_left, sizeof(ENTROPY_CONTEXT_PLANES));

            ta_s = (ENTROPY_CONTEXT *)&t_above_s;
            tl_s = (ENTROPY_CONTEXT *)&t_left_s;

            if (this_mode == NEW4X4)
            {
                int sseshift;
                int num00;
                int step_param = 0;
                int further_steps;
                int n;
                int thissme;
                int bestsme = INT_MAX;
                int_mv  temp_mv;
                BLOCK *c;
                BLOCKD *e;

                if (best_label_rd < label_mv_thresh)
                    break;

                if(cpi->compressor_speed)
                {
                    if (segmentation == BLOCK_8X16 || segmentation == BLOCK_16X8)
                    {
                        bsi->mvp.as_int = bsi->sv_mvp[i].as_int;
                        if (i==1 && segmentation == BLOCK_16X8)
                          bsi->mvp.as_int = bsi->sv_mvp[2].as_int;

                        step_param = bsi->sv_istep[i];
                    }

                    if (segmentation == BLOCK_4X4 && i>0)
                    {
                        bsi->mvp.as_int = x->e_mbd.block[i-1].bmi.mv.as_int;
                        if (i==4 || i==8 || i==12)
                            bsi->mvp.as_int = x->e_mbd.block[i-4].bmi.mv.as_int;
                        step_param = 2;
                    }
                }

                further_steps = (MAX_MVSEARCH_STEPS - 1) - step_param;

                {
                    int sadpb = x->sadperbit4;
                    int_mv mvp_full;

                    mvp_full.as_mv.row = bsi->mvp.as_mv.row >>3;
                    mvp_full.as_mv.col = bsi->mvp.as_mv.col >>3;

                    
                    n = vp8_mbsplit_offset[segmentation][i];

                    c = &x->block[n];
                    e = &x->e_mbd.block[n];

                    {
                        bestsme = cpi->diamond_search_sad(x, c, e, &mvp_full,
                                                &mode_mv[NEW4X4], step_param,
                                                sadpb, &num00, v_fn_ptr,
                                                x->mvcost, bsi->ref_mv);

                        n = num00;
                        num00 = 0;

                        while (n < further_steps)
                        {
                            n++;

                            if (num00)
                                num00--;
                            else
                            {
                                thissme = cpi->diamond_search_sad(x, c, e,
                                                    &mvp_full, &temp_mv,
                                                    step_param + n, sadpb,
                                                    &num00, v_fn_ptr,
                                                    x->mvcost, bsi->ref_mv);

                                if (thissme < bestsme)
                                {
                                    bestsme = thissme;
                                    mode_mv[NEW4X4].as_int = temp_mv.as_int;
                                }
                            }
                        }
                    }

                    sseshift = segmentation_to_sseshift[segmentation];

                    
                    if ((cpi->compressor_speed == 0) && (bestsme >> sseshift) > 4000)
                    {
                        
                        vp8_clamp_mv(&mvp_full, x->mv_col_min, x->mv_col_max, x->mv_row_min, x->mv_row_max);

                        thissme = cpi->full_search_sad(x, c, e, &mvp_full,
                                                       sadpb, 16, v_fn_ptr,
                                                       x->mvcost, bsi->ref_mv);

                        if (thissme < bestsme)
                        {
                            bestsme = thissme;
                            mode_mv[NEW4X4].as_int = e->bmi.mv.as_int;
                        }
                        else
                        {
                            e->bmi.mv.as_int = mode_mv[NEW4X4].as_int;
                        }
                    }
                }

                if (bestsme < INT_MAX)
                {
                    int disto;
                    unsigned int sse;
                    cpi->find_fractional_mv_step(x, c, e, &mode_mv[NEW4X4],
                        bsi->ref_mv, x->errorperbit, v_fn_ptr, x->mvcost,
                        &disto, &sse);
                }
            } 

            rate = labels2mode(x, labels, i, this_mode, &mode_mv[this_mode],
                               bsi->ref_mv, x->mvcost);

            
            if (((mode_mv[this_mode].as_mv.row >> 3) < x->mv_row_min) || ((mode_mv[this_mode].as_mv.row >> 3) > x->mv_row_max) ||
                ((mode_mv[this_mode].as_mv.col >> 3) < x->mv_col_min) || ((mode_mv[this_mode].as_mv.col >> 3) > x->mv_col_max))
            {
                continue;
            }

            distortion = vp8_encode_inter_mb_segment(x, labels, i) / 4;

            labelyrate = rdcost_mbsegment_y(x, labels, i, ta_s, tl_s);
            rate += labelyrate;

            this_rd = RDCOST(x->rdmult, x->rddiv, rate, distortion);

            if (this_rd < best_label_rd)
            {
                sbr = rate;
                sbd = distortion;
                bestlabelyrate = labelyrate;
                mode_selected = this_mode;
                best_label_rd = this_rd;

                memcpy(ta_b, ta_s, sizeof(ENTROPY_CONTEXT_PLANES));
                memcpy(tl_b, tl_s, sizeof(ENTROPY_CONTEXT_PLANES));

            }
        } 

        memcpy(ta, ta_b, sizeof(ENTROPY_CONTEXT_PLANES));
        memcpy(tl, tl_b, sizeof(ENTROPY_CONTEXT_PLANES));

        labels2mode(x, labels, i, mode_selected, &mode_mv[mode_selected],
                    bsi->ref_mv, x->mvcost);

        br += sbr;
        bd += sbd;
        segmentyrate += bestlabelyrate;
        this_segment_rd += best_label_rd;

        if (this_segment_rd >= bsi->segment_rd)
            break;

    } 

    if (this_segment_rd < bsi->segment_rd)
    {
        bsi->r = br;
        bsi->d = bd;
        bsi->segment_yrate = segmentyrate;
        bsi->segment_rd = this_segment_rd;
        bsi->segment_num = segmentation;

        
        for (i = 0; i < 16; i++)
        {
            bsi->mvs[i].as_mv = x->partition_info->bmi[i].mv.as_mv;
            bsi->modes[i] = x->partition_info->bmi[i].mode;
            bsi->eobs[i] = x->e_mbd.eobs[i];
        }
    }
}

static
void vp8_cal_step_param(int sr, int *sp)
{
    int step = 0;

    if (sr > MAX_FIRST_STEP) sr = MAX_FIRST_STEP;
    else if (sr < 1) sr = 1;

    while (sr>>=1)
        step++;

    *sp = MAX_MVSEARCH_STEPS - 1 - step;
}

static int vp8_rd_pick_best_mbsegmentation(VP8_COMP *cpi, MACROBLOCK *x,
                                           int_mv *best_ref_mv, int best_rd,
                                           int *mdcounts, int *returntotrate,
                                           int *returnyrate, int *returndistortion,
                                           int mvthresh)
{
    int i;
    BEST_SEG_INFO bsi;

    memset(&bsi, 0, sizeof(bsi));

    bsi.segment_rd = best_rd;
    bsi.ref_mv = best_ref_mv;
    bsi.mvp.as_int = best_ref_mv->as_int;
    bsi.mvthresh = mvthresh;
    bsi.mdcounts = mdcounts;

    for(i = 0; i < 16; i++)
    {
        bsi.modes[i] = ZERO4X4;
    }

    if(cpi->compressor_speed == 0)
    {
        rd_check_segment(cpi, x, &bsi, BLOCK_16X8);
        rd_check_segment(cpi, x, &bsi, BLOCK_8X16);
        rd_check_segment(cpi, x, &bsi, BLOCK_8X8);
        rd_check_segment(cpi, x, &bsi, BLOCK_4X4);
    }
    else
    {
        int sr;

        rd_check_segment(cpi, x, &bsi, BLOCK_8X8);

        if (bsi.segment_rd < best_rd)
        {
            int col_min = ((best_ref_mv->as_mv.col+7)>>3) - MAX_FULL_PEL_VAL;
            int row_min = ((best_ref_mv->as_mv.row+7)>>3) - MAX_FULL_PEL_VAL;
            int col_max = (best_ref_mv->as_mv.col>>3) + MAX_FULL_PEL_VAL;
            int row_max = (best_ref_mv->as_mv.row>>3) + MAX_FULL_PEL_VAL;

            int tmp_col_min = x->mv_col_min;
            int tmp_col_max = x->mv_col_max;
            int tmp_row_min = x->mv_row_min;
            int tmp_row_max = x->mv_row_max;

            
            if (x->mv_col_min < col_min )
                x->mv_col_min = col_min;
            if (x->mv_col_max > col_max )
                x->mv_col_max = col_max;
            if (x->mv_row_min < row_min )
                x->mv_row_min = row_min;
            if (x->mv_row_max > row_max )
                x->mv_row_max = row_max;

            
            bsi.sv_mvp[0].as_int = bsi.mvs[0].as_int;
            bsi.sv_mvp[1].as_int = bsi.mvs[2].as_int;
            bsi.sv_mvp[2].as_int = bsi.mvs[8].as_int;
            bsi.sv_mvp[3].as_int = bsi.mvs[10].as_int;

            
            
            {
                sr = MAXF((abs(bsi.sv_mvp[0].as_mv.row - bsi.sv_mvp[2].as_mv.row))>>3, (abs(bsi.sv_mvp[0].as_mv.col - bsi.sv_mvp[2].as_mv.col))>>3);
                vp8_cal_step_param(sr, &bsi.sv_istep[0]);

                sr = MAXF((abs(bsi.sv_mvp[1].as_mv.row - bsi.sv_mvp[3].as_mv.row))>>3, (abs(bsi.sv_mvp[1].as_mv.col - bsi.sv_mvp[3].as_mv.col))>>3);
                vp8_cal_step_param(sr, &bsi.sv_istep[1]);

                rd_check_segment(cpi, x, &bsi, BLOCK_8X16);
            }

            
            {
                sr = MAXF((abs(bsi.sv_mvp[0].as_mv.row - bsi.sv_mvp[1].as_mv.row))>>3, (abs(bsi.sv_mvp[0].as_mv.col - bsi.sv_mvp[1].as_mv.col))>>3);
                vp8_cal_step_param(sr, &bsi.sv_istep[0]);

                sr = MAXF((abs(bsi.sv_mvp[2].as_mv.row - bsi.sv_mvp[3].as_mv.row))>>3, (abs(bsi.sv_mvp[2].as_mv.col - bsi.sv_mvp[3].as_mv.col))>>3);
                vp8_cal_step_param(sr, &bsi.sv_istep[1]);

                rd_check_segment(cpi, x, &bsi, BLOCK_16X8);
            }

            
            
            if (cpi->sf.no_skip_block4x4_search || bsi.segment_num == BLOCK_8X8)  
            {
                bsi.mvp.as_int = bsi.sv_mvp[0].as_int;
                rd_check_segment(cpi, x, &bsi, BLOCK_4X4);
            }

            
            x->mv_col_min = tmp_col_min;
            x->mv_col_max = tmp_col_max;
            x->mv_row_min = tmp_row_min;
            x->mv_row_max = tmp_row_max;
        }
    }

    
    for (i = 0; i < 16; i++)
    {
        BLOCKD *bd = &x->e_mbd.block[i];

        bd->bmi.mv.as_int = bsi.mvs[i].as_int;
        *bd->eob = bsi.eobs[i];
    }

    *returntotrate = bsi.r;
    *returndistortion = bsi.d;
    *returnyrate = bsi.segment_yrate;

    
    x->e_mbd.mode_info_context->mbmi.partitioning = bsi.segment_num;
    x->partition_info->count = vp8_mbsplit_count[bsi.segment_num];

    for (i = 0; i < x->partition_info->count; i++)
    {
        int j;

        j = vp8_mbsplit_offset[bsi.segment_num][i];

        x->partition_info->bmi[i].mode = bsi.modes[j];
        x->partition_info->bmi[i].mv.as_mv = bsi.mvs[j].as_mv;
    }
    x->partition_info->bmi[15].mv.as_int = bsi.mvs[15].as_int;

    return bsi.segment_rd;
}

void vp8_mv_pred
(
    VP8_COMP *cpi,
    MACROBLOCKD *xd,
    const MODE_INFO *here,
    int_mv *mvp,
    int refframe,
    int *ref_frame_sign_bias,
    int *sr,
    int near_sadidx[]
)
{
    const MODE_INFO *above = here - xd->mode_info_stride;
    const MODE_INFO *left = here - 1;
    const MODE_INFO *aboveleft = above - 1;
    int_mv           near_mvs[8];
    int              near_ref[8];
    int_mv           mv;
    int              vcnt=0;
    int              find=0;
    int              mb_offset;

    int              mvx[8];
    int              mvy[8];
    int              i;

    mv.as_int = 0;

    if(here->mbmi.ref_frame != INTRA_FRAME)
    {
        near_mvs[0].as_int = near_mvs[1].as_int = near_mvs[2].as_int = near_mvs[3].as_int = near_mvs[4].as_int = near_mvs[5].as_int = near_mvs[6].as_int = near_mvs[7].as_int = 0;
        near_ref[0] = near_ref[1] = near_ref[2] = near_ref[3] = near_ref[4] = near_ref[5] = near_ref[6] = near_ref[7] = 0;

        if (above->mbmi.ref_frame != INTRA_FRAME)
        {
            near_mvs[vcnt].as_int = above->mbmi.mv.as_int;
            mv_bias(ref_frame_sign_bias[above->mbmi.ref_frame], refframe, &near_mvs[vcnt], ref_frame_sign_bias);
            near_ref[vcnt] =  above->mbmi.ref_frame;
        }
        vcnt++;
        if (left->mbmi.ref_frame != INTRA_FRAME)
        {
            near_mvs[vcnt].as_int = left->mbmi.mv.as_int;
            mv_bias(ref_frame_sign_bias[left->mbmi.ref_frame], refframe, &near_mvs[vcnt], ref_frame_sign_bias);
            near_ref[vcnt] =  left->mbmi.ref_frame;
        }
        vcnt++;
        if (aboveleft->mbmi.ref_frame != INTRA_FRAME)
        {
            near_mvs[vcnt].as_int = aboveleft->mbmi.mv.as_int;
            mv_bias(ref_frame_sign_bias[aboveleft->mbmi.ref_frame], refframe, &near_mvs[vcnt], ref_frame_sign_bias);
            near_ref[vcnt] =  aboveleft->mbmi.ref_frame;
        }
        vcnt++;

        
        if(cpi->common.last_frame_type != KEY_FRAME)
        {
            mb_offset = (-xd->mb_to_top_edge/128 + 1) * (xd->mode_info_stride +1) + (-xd->mb_to_left_edge/128 +1) ;

            
            if (cpi->lf_ref_frame[mb_offset] != INTRA_FRAME)
            {
                near_mvs[vcnt].as_int = cpi->lfmv[mb_offset].as_int;
                mv_bias(cpi->lf_ref_frame_sign_bias[mb_offset], refframe, &near_mvs[vcnt], ref_frame_sign_bias);
                near_ref[vcnt] =  cpi->lf_ref_frame[mb_offset];
            }
            vcnt++;

            
            if (cpi->lf_ref_frame[mb_offset - xd->mode_info_stride-1] != INTRA_FRAME)
            {
                near_mvs[vcnt].as_int = cpi->lfmv[mb_offset - xd->mode_info_stride-1].as_int;
                mv_bias(cpi->lf_ref_frame_sign_bias[mb_offset - xd->mode_info_stride-1], refframe, &near_mvs[vcnt], ref_frame_sign_bias);
                near_ref[vcnt] =  cpi->lf_ref_frame[mb_offset - xd->mode_info_stride-1];
            }
            vcnt++;

            
            if (cpi->lf_ref_frame[mb_offset-1] != INTRA_FRAME)
            {
                near_mvs[vcnt].as_int = cpi->lfmv[mb_offset -1].as_int;
                mv_bias(cpi->lf_ref_frame_sign_bias[mb_offset -1], refframe, &near_mvs[vcnt], ref_frame_sign_bias);
                near_ref[vcnt] =  cpi->lf_ref_frame[mb_offset - 1];
            }
            vcnt++;

            
            if (cpi->lf_ref_frame[mb_offset +1] != INTRA_FRAME)
            {
                near_mvs[vcnt].as_int = cpi->lfmv[mb_offset +1].as_int;
                mv_bias(cpi->lf_ref_frame_sign_bias[mb_offset +1], refframe, &near_mvs[vcnt], ref_frame_sign_bias);
                near_ref[vcnt] =  cpi->lf_ref_frame[mb_offset +1];
            }
            vcnt++;

            
            if (cpi->lf_ref_frame[mb_offset + xd->mode_info_stride +1] != INTRA_FRAME)
            {
                near_mvs[vcnt].as_int = cpi->lfmv[mb_offset + xd->mode_info_stride +1].as_int;
                mv_bias(cpi->lf_ref_frame_sign_bias[mb_offset + xd->mode_info_stride +1], refframe, &near_mvs[vcnt], ref_frame_sign_bias);
                near_ref[vcnt] =  cpi->lf_ref_frame[mb_offset + xd->mode_info_stride +1];
            }
            vcnt++;
        }

        for(i=0; i< vcnt; i++)
        {
            if(near_ref[near_sadidx[i]] != INTRA_FRAME)
            {
                if(here->mbmi.ref_frame == near_ref[near_sadidx[i]])
                {
                    mv.as_int = near_mvs[near_sadidx[i]].as_int;
                    find = 1;
                    if (i < 3)
                        *sr = 3;
                    else
                        *sr = 2;
                    break;
                }
            }
        }

        if(!find)
        {
            for(i=0; i<vcnt; i++)
            {
                mvx[i] = near_mvs[i].as_mv.row;
                mvy[i] = near_mvs[i].as_mv.col;
            }

            insertsortmv(mvx, vcnt);
            insertsortmv(mvy, vcnt);
            mv.as_mv.row = mvx[vcnt/2];
            mv.as_mv.col = mvy[vcnt/2];

            *sr = 0;
        }
    }

    
    mvp->as_int = mv.as_int;
    vp8_clamp_mv2(mvp, xd);
}

void vp8_cal_sad(VP8_COMP *cpi, MACROBLOCKD *xd, MACROBLOCK *x, int recon_yoffset, int near_sadidx[])
{
    int near_sad[8] = {0};
    BLOCK *b = &x->block[0];
    unsigned char *src_y_ptr = *(b->base_src);

    
    if( xd->mb_to_top_edge==0 && xd->mb_to_left_edge ==0)
    {
        near_sad[0] = near_sad[1] = near_sad[2] = INT_MAX;
    }else if(xd->mb_to_top_edge==0)
    {   
        near_sad[0] = near_sad[2] = INT_MAX;
        near_sad[1] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, xd->dst.y_buffer - 16,xd->dst.y_stride);
    }else if(xd->mb_to_left_edge ==0)
    {   
        near_sad[1] = near_sad[2] = INT_MAX;
        near_sad[0] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, xd->dst.y_buffer - xd->dst.y_stride *16,xd->dst.y_stride);
    }else
    {
        near_sad[0] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, xd->dst.y_buffer - xd->dst.y_stride *16,xd->dst.y_stride);
        near_sad[1] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, xd->dst.y_buffer - 16,xd->dst.y_stride);
        near_sad[2] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, xd->dst.y_buffer - xd->dst.y_stride *16 -16,xd->dst.y_stride);
    }

    if(cpi->common.last_frame_type != KEY_FRAME)
    {
        
        unsigned char *pre_y_buffer = cpi->common.yv12_fb[cpi->common.lst_fb_idx].y_buffer + recon_yoffset;
        int pre_y_stride = cpi->common.yv12_fb[cpi->common.lst_fb_idx].y_stride;

        if(xd->mb_to_top_edge==0) near_sad[4] = INT_MAX;
        if(xd->mb_to_left_edge ==0) near_sad[5] = INT_MAX;
        if(xd->mb_to_right_edge ==0) near_sad[6] = INT_MAX;
        if(xd->mb_to_bottom_edge==0) near_sad[7] = INT_MAX;

        if(near_sad[4] != INT_MAX)
            near_sad[4] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, pre_y_buffer - pre_y_stride *16, pre_y_stride);
        if(near_sad[5] != INT_MAX)
            near_sad[5] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, pre_y_buffer - 16, pre_y_stride);
        near_sad[3] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, pre_y_buffer, pre_y_stride);
        if(near_sad[6] != INT_MAX)
            near_sad[6] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, pre_y_buffer + 16, pre_y_stride);
        if(near_sad[7] != INT_MAX)
            near_sad[7] = cpi->fn_ptr[BLOCK_16X16].sdf(src_y_ptr, b->src_stride, pre_y_buffer + pre_y_stride *16, pre_y_stride);
    }

    if(cpi->common.last_frame_type != KEY_FRAME)
    {
        insertsortsad(near_sad, near_sadidx, 8);
    }else
    {
        insertsortsad(near_sad, near_sadidx, 3);
    }
}

static void rd_update_mvcount(MACROBLOCK *x, int_mv *best_ref_mv)
{
    if (x->e_mbd.mode_info_context->mbmi.mode == SPLITMV)
    {
        int i;

        for (i = 0; i < x->partition_info->count; i++)
        {
            if (x->partition_info->bmi[i].mode == NEW4X4)
            {
                x->MVcount[0][mv_max+((x->partition_info->bmi[i].mv.as_mv.row
                                          - best_ref_mv->as_mv.row) >> 1)]++;
                x->MVcount[1][mv_max+((x->partition_info->bmi[i].mv.as_mv.col
                                          - best_ref_mv->as_mv.col) >> 1)]++;
            }
        }
    }
    else if (x->e_mbd.mode_info_context->mbmi.mode == NEWMV)
    {
        x->MVcount[0][mv_max+((x->e_mbd.mode_info_context->mbmi.mv.as_mv.row
                                          - best_ref_mv->as_mv.row) >> 1)]++;
        x->MVcount[1][mv_max+((x->e_mbd.mode_info_context->mbmi.mv.as_mv.col
                                          - best_ref_mv->as_mv.col) >> 1)]++;
    }
}

static int evaluate_inter_mode_rd(int mdcounts[4],
                                  RATE_DISTORTION* rd,
                                  int* disable_skip,
                                  VP8_COMP *cpi, MACROBLOCK *x)
{
    MB_PREDICTION_MODE this_mode = x->e_mbd.mode_info_context->mbmi.mode;
    BLOCK *b = &x->block[0];
    MACROBLOCKD *xd = &x->e_mbd;
    int distortion;
    vp8_build_inter16x16_predictors_mby(&x->e_mbd, x->e_mbd.predictor, 16);

    if (cpi->active_map_enabled && x->active_ptr[0] == 0) {
        x->skip = 1;
    }
    else if (x->encode_breakout)
    {
        unsigned int sse;
        unsigned int var;
        unsigned int threshold = (xd->block[0].dequant[1]
                    * xd->block[0].dequant[1] >>4);

        if(threshold < x->encode_breakout)
            threshold = x->encode_breakout;

        var = vpx_variance16x16
                (*(b->base_src), b->src_stride,
                x->e_mbd.predictor, 16, &sse);

        if (sse < threshold)
        {
             unsigned int q2dc = xd->block[24].dequant[0];
            if ((sse - var < q2dc * q2dc >>4) ||
                (sse /2 > var && sse-var < 64))
            {
                
                unsigned int sse2 = VP8_UVSSE(x);
                if (sse2 * 2 < threshold)
                {
                    x->skip = 1;
                    rd->distortion2 = sse + sse2;
                    rd->rate2 = 500;

                    
                    rd->rate_uv = 0;
                    rd->distortion_uv = sse2;

                    *disable_skip = 1;
                    return RDCOST(x->rdmult, x->rddiv, rd->rate2,
                                  rd->distortion2);
                }
            }
        }
    }


    
    rd->rate2 += vp8_cost_mv_ref(this_mode, mdcounts);

    
    macro_block_yrd(x, &rd->rate_y, &distortion);
    rd->rate2 += rd->rate_y;
    rd->distortion2 += distortion;

    
    rd_inter16x16_uv(cpi, x, &rd->rate_uv, &rd->distortion_uv,
                     cpi->common.full_pixel);
    rd->rate2 += rd->rate_uv;
    rd->distortion2 += rd->distortion_uv;
    return INT_MAX;
}

static int calculate_final_rd_costs(int this_rd,
                                    RATE_DISTORTION* rd,
                                    int* other_cost,
                                    int disable_skip,
                                    int uv_intra_tteob,
                                    int intra_rd_penalty,
                                    VP8_COMP *cpi, MACROBLOCK *x)
{
    MB_PREDICTION_MODE this_mode = x->e_mbd.mode_info_context->mbmi.mode;

    if (cpi->common.mb_no_coeff_skip)
    {
        *other_cost += vp8_cost_bit(cpi->prob_skip_false, 0);
        rd->rate2 += *other_cost;
    }

    rd->rate2 +=
        x->ref_frame_cost[x->e_mbd.mode_info_context->mbmi.ref_frame];

    if (!disable_skip)
    {
        if (cpi->common.mb_no_coeff_skip)
        {
            int i;
            int tteob;
            int has_y2_block = (this_mode!=SPLITMV && this_mode!=B_PRED);

            tteob = 0;
            if(has_y2_block)
                tteob += x->e_mbd.eobs[24];

            for (i = 0; i < 16; i++)
                tteob += (x->e_mbd.eobs[i] > has_y2_block);

            if (x->e_mbd.mode_info_context->mbmi.ref_frame)
            {
                for (i = 16; i < 24; i++)
                    tteob += x->e_mbd.eobs[i];
            }
            else
                tteob += uv_intra_tteob;

            if (tteob == 0)
            {
                rd->rate2 -= (rd->rate_y + rd->rate_uv);
                
                rd->rate_uv = 0;

                
                if (cpi->prob_skip_false)
                {
                    int prob_skip_cost;

                    prob_skip_cost = vp8_cost_bit(cpi->prob_skip_false, 1);
                    prob_skip_cost -= vp8_cost_bit(cpi->prob_skip_false, 0);
                    rd->rate2 += prob_skip_cost;
                    *other_cost += prob_skip_cost;
                }
            }
        }
        
        this_rd = RDCOST(x->rdmult, x->rddiv, rd->rate2, rd->distortion2);
        if (this_rd < INT_MAX && x->e_mbd.mode_info_context->mbmi.ref_frame
                                 == INTRA_FRAME)
            this_rd += intra_rd_penalty;
    }
    return this_rd;
}

static void update_best_mode(BEST_MODE* best_mode, int this_rd,
                             RATE_DISTORTION* rd, int other_cost, MACROBLOCK *x)
{
    MB_PREDICTION_MODE this_mode = x->e_mbd.mode_info_context->mbmi.mode;

    other_cost +=
    x->ref_frame_cost[x->e_mbd.mode_info_context->mbmi.ref_frame];

    
    best_mode->yrd = RDCOST(x->rdmult, x->rddiv, (rd->rate2-rd->rate_uv-other_cost),
                      (rd->distortion2-rd->distortion_uv));

    best_mode->rd = this_rd;
    memcpy(&best_mode->mbmode, &x->e_mbd.mode_info_context->mbmi, sizeof(MB_MODE_INFO));
    memcpy(&best_mode->partition, x->partition_info, sizeof(PARTITION_INFO));

    if ((this_mode == B_PRED) || (this_mode == SPLITMV))
    {
        int i;
        for (i = 0; i < 16; i++)
        {
            best_mode->bmodes[i] = x->e_mbd.block[i].bmi;
        }
    }
}

void vp8_rd_pick_inter_mode(VP8_COMP *cpi, MACROBLOCK *x, int recon_yoffset,
                            int recon_uvoffset, int *returnrate,
                            int *returndistortion, int *returnintra,
                            int mb_row, int mb_col)
{
    BLOCK *b = &x->block[0];
    BLOCKD *d = &x->e_mbd.block[0];
    MACROBLOCKD *xd = &x->e_mbd;
    int_mv best_ref_mv_sb[2];
    int_mv mode_mv_sb[2][MB_MODE_COUNT];
    int_mv best_ref_mv;
    int_mv *mode_mv;
    MB_PREDICTION_MODE this_mode;
    int num00;
    int best_mode_index = 0;
    BEST_MODE best_mode;

    int i;
    int mode_index;
    int mdcounts[4];
    int rate;
    RATE_DISTORTION rd;
    int uv_intra_rate, uv_intra_distortion, uv_intra_rate_tokenonly;
    int uv_intra_tteob = 0;
    int uv_intra_done = 0;

    MB_PREDICTION_MODE uv_intra_mode = 0;
    int_mv mvp;
    int near_sadidx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int saddone=0;
    
    int sr=0;

    unsigned char *plane[4][3];
    int ref_frame_map[4];
    int sign_bias = 0;

    int intra_rd_penalty =  10* vp8_dc_quant(cpi->common.base_qindex,
                                             cpi->common.y1dc_delta_q);

#if CONFIG_TEMPORAL_DENOISING
    unsigned int zero_mv_sse = UINT_MAX, best_sse = UINT_MAX,
            best_rd_sse = UINT_MAX;
#endif

    mode_mv = mode_mv_sb[sign_bias];
    best_ref_mv.as_int = 0;
    best_mode.rd = INT_MAX;
    best_mode.yrd = INT_MAX;
    best_mode.intra_rd = INT_MAX;
    memset(mode_mv_sb, 0, sizeof(mode_mv_sb));
    memset(&best_mode.mbmode, 0, sizeof(best_mode.mbmode));
    memset(&best_mode.bmodes, 0, sizeof(best_mode.bmodes));

    
    get_reference_search_order(cpi, ref_frame_map);

    if (ref_frame_map[1] > 0)
    {
        sign_bias = vp8_find_near_mvs_bias(&x->e_mbd,
                                           x->e_mbd.mode_info_context,
                                           mode_mv_sb,
                                           best_ref_mv_sb,
                                           mdcounts,
                                           ref_frame_map[1],
                                           cpi->common.ref_frame_sign_bias);

        mode_mv = mode_mv_sb[sign_bias];
        best_ref_mv.as_int = best_ref_mv_sb[sign_bias].as_int;
    }

    get_predictor_pointers(cpi, plane, recon_yoffset, recon_uvoffset);

    *returnintra = INT_MAX;
    
    x->mbs_tested_so_far++;

    x->skip = 0;

    for (mode_index = 0; mode_index < MAX_MODES; mode_index++)
    {
        int this_rd = INT_MAX;
        int disable_skip = 0;
        int other_cost = 0;
        int this_ref_frame = ref_frame_map[vp8_ref_frame_order[mode_index]];

        
        if (best_mode.rd <= x->rd_threshes[mode_index])
            continue;

        if (this_ref_frame < 0)
            continue;

        rd.rate2 = 0;
        rd.distortion2 = 0;

        this_mode = vp8_mode_order[mode_index];

        x->e_mbd.mode_info_context->mbmi.mode = this_mode;
        x->e_mbd.mode_info_context->mbmi.ref_frame = this_ref_frame;

        if (cpi->is_src_frame_alt_ref && (cpi->oxcf.arnr_max_frames == 0))
        {
            if (this_mode != ZEROMV || x->e_mbd.mode_info_context->mbmi.ref_frame != ALTREF_FRAME)
                continue;
        }

        
        if (x->e_mbd.mode_info_context->mbmi.ref_frame)
        {
            x->e_mbd.pre.y_buffer = plane[this_ref_frame][0];
            x->e_mbd.pre.u_buffer = plane[this_ref_frame][1];
            x->e_mbd.pre.v_buffer = plane[this_ref_frame][2];

            if (sign_bias != cpi->common.ref_frame_sign_bias[this_ref_frame])
            {
                sign_bias = cpi->common.ref_frame_sign_bias[this_ref_frame];
                mode_mv = mode_mv_sb[sign_bias];
                best_ref_mv.as_int = best_ref_mv_sb[sign_bias].as_int;
            }
        }

        if (x->mode_test_hit_counts[mode_index] && (cpi->mode_check_freq[mode_index] > 1))
        {
            if (x->mbs_tested_so_far  <= cpi->mode_check_freq[mode_index] * x->mode_test_hit_counts[mode_index])
            {
                x->rd_thresh_mult[mode_index] += 4;

                if (x->rd_thresh_mult[mode_index] > MAX_THRESHMULT)
                    x->rd_thresh_mult[mode_index] = MAX_THRESHMULT;

                x->rd_threshes[mode_index] =
                    (cpi->rd_baseline_thresh[mode_index] >> 7) *
                    x->rd_thresh_mult[mode_index];

                continue;
            }
        }

        x->mode_test_hit_counts[mode_index] ++;

        if (x->zbin_mode_boost_enabled)
        {
            if ( this_ref_frame == INTRA_FRAME )
                x->zbin_mode_boost = 0;
            else
            {
                if (vp8_mode_order[mode_index] == ZEROMV)
                {
                    if (this_ref_frame != LAST_FRAME)
                        x->zbin_mode_boost = GF_ZEROMV_ZBIN_BOOST;
                    else
                        x->zbin_mode_boost = LF_ZEROMV_ZBIN_BOOST;
                }
                else if (vp8_mode_order[mode_index] == SPLITMV)
                    x->zbin_mode_boost = 0;
                else
                    x->zbin_mode_boost = MV_ZBIN_BOOST;
            }

            vp8_update_zbin_extra(cpi, x);
        }

        if(!uv_intra_done && this_ref_frame == INTRA_FRAME)
        {
            rd_pick_intra_mbuv_mode(x, &uv_intra_rate,
                                    &uv_intra_rate_tokenonly,
                                    &uv_intra_distortion);
            uv_intra_mode = x->e_mbd.mode_info_context->mbmi.uv_mode;

            /*
             * Total of the eobs is used later to further adjust rate2. Since uv
             * block's intra eobs will be overwritten when we check inter modes,
             * we need to save uv_intra_tteob here.
             */
            for (i = 16; i < 24; i++)
                uv_intra_tteob += x->e_mbd.eobs[i];

            uv_intra_done = 1;
        }

        switch (this_mode)
        {
        case B_PRED:
        {
            int tmp_rd;

            int distortion;
            tmp_rd = rd_pick_intra4x4mby_modes(x, &rate, &rd.rate_y, &distortion, best_mode.yrd);
            rd.rate2 += rate;
            rd.distortion2 += distortion;

            if(tmp_rd < best_mode.yrd)
            {
                rd.rate2 += uv_intra_rate;
                rd.rate_uv = uv_intra_rate_tokenonly;
                rd.distortion2 += uv_intra_distortion;
                rd.distortion_uv = uv_intra_distortion;
            }
            else
            {
                this_rd = INT_MAX;
                disable_skip = 1;
            }
        }
        break;

        case SPLITMV:
        {
            int tmp_rd;
            int this_rd_thresh;
            int distortion;

            this_rd_thresh = (vp8_ref_frame_order[mode_index] == 1) ?
                x->rd_threshes[THR_NEW1] : x->rd_threshes[THR_NEW3];
            this_rd_thresh = (vp8_ref_frame_order[mode_index] == 2) ?
                x->rd_threshes[THR_NEW2] : this_rd_thresh;

            tmp_rd = vp8_rd_pick_best_mbsegmentation(cpi, x, &best_ref_mv,
                                                     best_mode.yrd, mdcounts,
                                                     &rate, &rd.rate_y, &distortion, this_rd_thresh) ;

            rd.rate2 += rate;
            rd.distortion2 += distortion;

            if (tmp_rd < best_mode.yrd)
            {
                
                rd_inter4x4_uv(cpi, x, &rd.rate_uv, &rd.distortion_uv, cpi->common.full_pixel);
                rd.rate2 += rd.rate_uv;
                rd.distortion2 += rd.distortion_uv;
            }
            else
            {
                this_rd = INT_MAX;
                disable_skip = 1;
            }
        }
        break;
        case DC_PRED:
        case V_PRED:
        case H_PRED:
        case TM_PRED:
        {
            int distortion;
            x->e_mbd.mode_info_context->mbmi.ref_frame = INTRA_FRAME;

            vp8_build_intra_predictors_mby_s(xd,
                                             xd->dst.y_buffer - xd->dst.y_stride,
                                             xd->dst.y_buffer - 1,
                                             xd->dst.y_stride,
                                             xd->predictor,
                                             16);
            macro_block_yrd(x, &rd.rate_y, &distortion) ;
            rd.rate2 += rd.rate_y;
            rd.distortion2 += distortion;
            rd.rate2 += x->mbmode_cost[x->e_mbd.frame_type][x->e_mbd.mode_info_context->mbmi.mode];
            rd.rate2 += uv_intra_rate;
            rd.rate_uv = uv_intra_rate_tokenonly;
            rd.distortion2 += uv_intra_distortion;
            rd.distortion_uv = uv_intra_distortion;
        }
        break;

        case NEWMV:
        {
            int thissme;
            int bestsme = INT_MAX;
            int step_param = cpi->sf.first_step;
            int further_steps;
            int n;
            int do_refine=1;   

            int sadpb = x->sadperbit16;
            int_mv mvp_full;

            int col_min = ((best_ref_mv.as_mv.col+7)>>3) - MAX_FULL_PEL_VAL;
            int row_min = ((best_ref_mv.as_mv.row+7)>>3) - MAX_FULL_PEL_VAL;
            int col_max = (best_ref_mv.as_mv.col>>3) + MAX_FULL_PEL_VAL;
            int row_max = (best_ref_mv.as_mv.row>>3) + MAX_FULL_PEL_VAL;

            int tmp_col_min = x->mv_col_min;
            int tmp_col_max = x->mv_col_max;
            int tmp_row_min = x->mv_row_min;
            int tmp_row_max = x->mv_row_max;

            if(!saddone)
            {
                vp8_cal_sad(cpi,xd,x, recon_yoffset ,&near_sadidx[0] );
                saddone = 1;
            }

            vp8_mv_pred(cpi, &x->e_mbd, x->e_mbd.mode_info_context, &mvp,
                        x->e_mbd.mode_info_context->mbmi.ref_frame, cpi->common.ref_frame_sign_bias, &sr, &near_sadidx[0]);

            mvp_full.as_mv.col = mvp.as_mv.col>>3;
            mvp_full.as_mv.row = mvp.as_mv.row>>3;

            if (x->mv_col_min < col_min )
                x->mv_col_min = col_min;
            if (x->mv_col_max > col_max )
                x->mv_col_max = col_max;
            if (x->mv_row_min < row_min )
                x->mv_row_min = row_min;
            if (x->mv_row_max > row_max )
                x->mv_row_max = row_max;

            
            if(sr > step_param)
                step_param = sr;

            
            {
                bestsme = cpi->diamond_search_sad(x, b, d, &mvp_full, &d->bmi.mv,
                                        step_param, sadpb, &num00,
                                        &cpi->fn_ptr[BLOCK_16X16],
                                        x->mvcost, &best_ref_mv);
                mode_mv[NEWMV].as_int = d->bmi.mv.as_int;

                
                further_steps = (cpi->sf.max_step_search_steps - 1) - step_param;

                n = num00;
                num00 = 0;

                
                if (n > further_steps)
                    do_refine = 0;

                while (n < further_steps)
                {
                    n++;

                    if (num00)
                        num00--;
                    else
                    {
                        thissme = cpi->diamond_search_sad(x, b, d, &mvp_full,
                                    &d->bmi.mv, step_param + n, sadpb, &num00,
                                    &cpi->fn_ptr[BLOCK_16X16], x->mvcost,
                                    &best_ref_mv);

                        
                        if (num00 > (further_steps-n))
                            do_refine = 0;

                        if (thissme < bestsme)
                        {
                            bestsme = thissme;
                            mode_mv[NEWMV].as_int = d->bmi.mv.as_int;
                        }
                        else
                        {
                            d->bmi.mv.as_int = mode_mv[NEWMV].as_int;
                        }
                    }
                }
            }

            
            if (do_refine == 1)
            {
                int search_range;

                search_range = 8;

                thissme = cpi->refining_search_sad(x, b, d, &d->bmi.mv, sadpb,
                                       search_range, &cpi->fn_ptr[BLOCK_16X16],
                                       x->mvcost, &best_ref_mv);

                if (thissme < bestsme)
                {
                    bestsme = thissme;
                    mode_mv[NEWMV].as_int = d->bmi.mv.as_int;
                }
                else
                {
                    d->bmi.mv.as_int = mode_mv[NEWMV].as_int;
                }
            }

            x->mv_col_min = tmp_col_min;
            x->mv_col_max = tmp_col_max;
            x->mv_row_min = tmp_row_min;
            x->mv_row_max = tmp_row_max;

            if (bestsme < INT_MAX)
            {
                int dis; 
                unsigned int sse;
                cpi->find_fractional_mv_step(x, b, d, &d->bmi.mv, &best_ref_mv,
                                             x->errorperbit,
                                             &cpi->fn_ptr[BLOCK_16X16],
                                             x->mvcost, &dis, &sse);
            }

            mode_mv[NEWMV].as_int = d->bmi.mv.as_int;

            
            rd.rate2 += vp8_mv_bit_cost(&mode_mv[NEWMV], &best_ref_mv, x->mvcost, 96);
        }

        case NEARESTMV:
        case NEARMV:
            vp8_clamp_mv2(&mode_mv[this_mode], xd);

            if (((this_mode == NEARMV) || (this_mode == NEARESTMV)) && (mode_mv[this_mode].as_int == 0))
                continue;

        case ZEROMV:

            if (((mode_mv[this_mode].as_mv.row >> 3) < x->mv_row_min) || ((mode_mv[this_mode].as_mv.row >> 3) > x->mv_row_max) ||
                ((mode_mv[this_mode].as_mv.col >> 3) < x->mv_col_min) || ((mode_mv[this_mode].as_mv.col >> 3) > x->mv_col_max))
                continue;

            vp8_set_mbmode_and_mvs(x, this_mode, &mode_mv[this_mode]);
            this_rd = evaluate_inter_mode_rd(mdcounts, &rd,
                                             &disable_skip, cpi, x);
            break;

        default:
            break;
        }

        this_rd = calculate_final_rd_costs(this_rd, &rd, &other_cost,
                                           disable_skip, uv_intra_tteob,
                                           intra_rd_penalty, cpi, x);

        
        if ((x->e_mbd.mode_info_context->mbmi.ref_frame == INTRA_FRAME) &&
            (this_rd < best_mode.intra_rd) )
        {
          best_mode.intra_rd = this_rd;
            *returnintra = rd.distortion2 ;
        }
#if CONFIG_TEMPORAL_DENOISING
        if (cpi->oxcf.noise_sensitivity)
        {
            unsigned int sse;
            vp8_get_inter_mbpred_error(x,&cpi->fn_ptr[BLOCK_16X16],&sse,
                                   mode_mv[this_mode]);

            if (sse < best_rd_sse)
                best_rd_sse = sse;

            
            if (this_mode == ZEROMV && sse < zero_mv_sse )
            {
                zero_mv_sse = sse;
                x->best_zeromv_reference_frame =
                        x->e_mbd.mode_info_context->mbmi.ref_frame;
            }

            
            if (x->e_mbd.mode_info_context->mbmi.mode == NEWMV &&
                    sse < best_sse)
            {
                best_sse = sse;
                vp8_get_inter_mbpred_error(x,&cpi->fn_ptr[BLOCK_16X16],&best_sse,
                                       mode_mv[this_mode]);
                x->best_sse_inter_mode = NEWMV;
                x->best_sse_mv = x->e_mbd.mode_info_context->mbmi.mv;
                x->need_to_clamp_best_mvs =
                    x->e_mbd.mode_info_context->mbmi.need_to_clamp_mvs;
                x->best_reference_frame =
                    x->e_mbd.mode_info_context->mbmi.ref_frame;
            }
        }
#endif

        
        if (this_rd < best_mode.rd || x->skip)
        {
            
            best_mode_index = mode_index;
            *returnrate = rd.rate2;
            *returndistortion = rd.distortion2;
            if (this_mode <= B_PRED)
            {
                x->e_mbd.mode_info_context->mbmi.uv_mode = uv_intra_mode;
                
                x->e_mbd.mode_info_context->mbmi.mv.as_int = 0;
            }
            update_best_mode(&best_mode, this_rd, &rd, other_cost, x);


            x->rd_thresh_mult[mode_index] =
                (x->rd_thresh_mult[mode_index] >= (MIN_THRESHMULT + 2)) ?
                    x->rd_thresh_mult[mode_index] - 2 : MIN_THRESHMULT;
        }

        else
        {
            x->rd_thresh_mult[mode_index] += 4;

            if (x->rd_thresh_mult[mode_index] > MAX_THRESHMULT)
                x->rd_thresh_mult[mode_index] = MAX_THRESHMULT;
        }
        x->rd_threshes[mode_index] =
            (cpi->rd_baseline_thresh[mode_index] >> 7) *
                x->rd_thresh_mult[mode_index];

        if (x->skip)
            break;

    }

    
    if ((cpi->rd_baseline_thresh[best_mode_index] > 0) && (cpi->rd_baseline_thresh[best_mode_index] < (INT_MAX >> 2)))
    {
        int best_adjustment = (x->rd_thresh_mult[best_mode_index] >> 2);

        x->rd_thresh_mult[best_mode_index] =
            (x->rd_thresh_mult[best_mode_index] >=
                (MIN_THRESHMULT + best_adjustment)) ?
                    x->rd_thresh_mult[best_mode_index] - best_adjustment :
                    MIN_THRESHMULT;
        x->rd_threshes[best_mode_index] =
            (cpi->rd_baseline_thresh[best_mode_index] >> 7) *
                x->rd_thresh_mult[best_mode_index];
    }

#if CONFIG_TEMPORAL_DENOISING
    if (cpi->oxcf.noise_sensitivity)
    {
        int block_index = mb_row * cpi->common.mb_cols + mb_col;
        if (x->best_sse_inter_mode == DC_PRED)
        {
            
            x->best_sse_inter_mode = best_mode.mbmode.mode;
            x->best_sse_mv = best_mode.mbmode.mv;
            x->need_to_clamp_best_mvs = best_mode.mbmode.need_to_clamp_mvs;
            x->best_reference_frame = best_mode.mbmode.ref_frame;
            best_sse = best_rd_sse;
        }
        vp8_denoiser_denoise_mb(&cpi->denoiser, x, best_sse, zero_mv_sse,
                                recon_yoffset, recon_uvoffset,
                                &cpi->common.lf_info, mb_row, mb_col,
                                block_index);

        
        if (best_mode.mbmode.ref_frame == INTRA_FRAME &&
            x->best_zeromv_reference_frame != INTRA_FRAME)
        {
            int this_rd = INT_MAX;
            int disable_skip = 0;
            int other_cost = 0;
            int this_ref_frame = x->best_zeromv_reference_frame;
            rd.rate2 = x->ref_frame_cost[this_ref_frame] +
                    vp8_cost_mv_ref(ZEROMV, mdcounts);
            rd.distortion2 = 0;

            
            x->e_mbd.mode_info_context->mbmi.ref_frame = this_ref_frame;
            x->e_mbd.pre.y_buffer = plane[this_ref_frame][0];
            x->e_mbd.pre.u_buffer = plane[this_ref_frame][1];
            x->e_mbd.pre.v_buffer = plane[this_ref_frame][2];

            x->e_mbd.mode_info_context->mbmi.mode = ZEROMV;
            x->e_mbd.mode_info_context->mbmi.uv_mode = DC_PRED;
            x->e_mbd.mode_info_context->mbmi.mv.as_int = 0;

            this_rd = evaluate_inter_mode_rd(mdcounts, &rd, &disable_skip, cpi, x);
            this_rd = calculate_final_rd_costs(this_rd, &rd, &other_cost,
                                               disable_skip, uv_intra_tteob,
                                               intra_rd_penalty, cpi, x);
            if (this_rd < best_mode.rd || x->skip)
            {
                *returnrate = rd.rate2;
                *returndistortion = rd.distortion2;
                update_best_mode(&best_mode, this_rd, &rd, other_cost, x);
            }
        }

    }
#endif

    if (cpi->is_src_frame_alt_ref &&
        (best_mode.mbmode.mode != ZEROMV || best_mode.mbmode.ref_frame != ALTREF_FRAME))
    {
        x->e_mbd.mode_info_context->mbmi.mode = ZEROMV;
        x->e_mbd.mode_info_context->mbmi.ref_frame = ALTREF_FRAME;
        x->e_mbd.mode_info_context->mbmi.mv.as_int = 0;
        x->e_mbd.mode_info_context->mbmi.uv_mode = DC_PRED;
        x->e_mbd.mode_info_context->mbmi.mb_skip_coeff =
                                        (cpi->common.mb_no_coeff_skip);
        x->e_mbd.mode_info_context->mbmi.partitioning = 0;
        return;
    }


    
    memcpy(&x->e_mbd.mode_info_context->mbmi, &best_mode.mbmode, sizeof(MB_MODE_INFO));

    if (best_mode.mbmode.mode == B_PRED)
    {
        for (i = 0; i < 16; i++)
            xd->mode_info_context->bmi[i].as_mode = best_mode.bmodes[i].as_mode;
    }

    if (best_mode.mbmode.mode == SPLITMV)
    {
        for (i = 0; i < 16; i++)
            xd->mode_info_context->bmi[i].mv.as_int = best_mode.bmodes[i].mv.as_int;

        memcpy(x->partition_info, &best_mode.partition, sizeof(PARTITION_INFO));

        x->e_mbd.mode_info_context->mbmi.mv.as_int =
                                      x->partition_info->bmi[15].mv.as_int;
    }

    if (sign_bias
        != cpi->common.ref_frame_sign_bias[xd->mode_info_context->mbmi.ref_frame])
        best_ref_mv.as_int = best_ref_mv_sb[!sign_bias].as_int;

    rd_update_mvcount(x, &best_ref_mv);
}

void vp8_rd_pick_intra_mode(MACROBLOCK *x, int *rate_)
{
    int error4x4, error16x16;
    int rate4x4, rate16x16 = 0, rateuv;
    int dist4x4, dist16x16, distuv;
    int rate;
    int rate4x4_tokenonly = 0;
    int rate16x16_tokenonly = 0;
    int rateuv_tokenonly = 0;

    x->e_mbd.mode_info_context->mbmi.ref_frame = INTRA_FRAME;

    rd_pick_intra_mbuv_mode(x, &rateuv, &rateuv_tokenonly, &distuv);
    rate = rateuv;

    error16x16 = rd_pick_intra16x16mby_mode(x, &rate16x16, &rate16x16_tokenonly,
                                            &dist16x16);

    error4x4 = rd_pick_intra4x4mby_modes(x, &rate4x4, &rate4x4_tokenonly,
                                         &dist4x4, error16x16);

    if (error4x4 < error16x16)
    {
        x->e_mbd.mode_info_context->mbmi.mode = B_PRED;
        rate += rate4x4;
    }
    else
    {
        rate += rate16x16;
    }

    *rate_ = rate;
}
