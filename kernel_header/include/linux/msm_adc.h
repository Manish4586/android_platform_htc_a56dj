#ifndef __MSM_ADC_H
#define __MSM_ADC_H

#include <linux/sched.h>

#define MSM_ADC_MAX_CHAN_STR 64

#define MSM_ADC_DEV_MAX_INFLIGHT 9

#define MSM_ADC_IOCTL_CODE		0x90

struct msm_adc_conversion {
	
	uint32_t chan;
	
	int result;
};

struct adc_chan_result {
	
	uint32_t chan;
	int32_t adc_code;
	int64_t measurement;
	int64_t physical;
};

#define MSM_ADC_REQUEST			_IOWR(MSM_ADC_IOCTL_CODE, 1,	\
					     struct adc_chan_result)

#define MSM_ADC_AIO_REQUEST		_IOWR(MSM_ADC_IOCTL_CODE, 2,	\
					     struct adc_chan_result)

#define MSM_ADC_AIO_REQUEST_BLOCK_RES	_IOWR(MSM_ADC_IOCTL_CODE, 3,	\
					     struct adc_chan_result)

#define MSM_ADC_AIO_POLL		_IOR(MSM_ADC_IOCTL_CODE, 4,	\
					     uint32_t)

#define MSM_ADC_FLUID_INIT	_IOR(MSM_ADC_IOCTL_CODE, 5,	\
					     uint32_t)

#define MSM_ADC_FLUID_DEINIT	_IOR(MSM_ADC_IOCTL_CODE, 6,	\
					     uint32_t)

struct msm_adc_aio_result {
	uint32_t chan;
	int result;
};

#define MSM_ADC_AIO_READ		_IOR(MSM_ADC_IOCTL_CODE, 5,	\
					     struct adc_chan_result)

struct msm_adc_lookup {
	
	char name[MSM_ADC_MAX_CHAN_STR];
	
	uint32_t chan_idx;
};

#define MSM_ADC_LOOKUP			_IOWR(MSM_ADC_IOCTL_CODE, 6,	\
					     struct msm_adc_lookup)


#endif 
