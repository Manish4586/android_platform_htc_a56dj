#ifndef __SOUND_SB16_CSP_H
#define __SOUND_SB16_CSP_H

/*
 *  Copyright (c) 1999 by Uros Bizjak <uros@kss-loka.si>
 *                        Takashi Iwai <tiwai@suse.de>
 *
 *  SB16ASP/AWE32 CSP control
 *
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#define SNDRV_SB_CSP_MODE_NONE		0x00
#define SNDRV_SB_CSP_MODE_DSP_READ	0x01	
#define SNDRV_SB_CSP_MODE_DSP_WRITE	0x02	
#define SNDRV_SB_CSP_MODE_QSOUND		0x04	

#define SNDRV_SB_CSP_LOAD_FROMUSER	0x01
#define SNDRV_SB_CSP_LOAD_INITBLOCK	0x02

#define SNDRV_SB_CSP_SAMPLE_8BIT		0x01
#define SNDRV_SB_CSP_SAMPLE_16BIT		0x02

#define SNDRV_SB_CSP_MONO			0x01
#define SNDRV_SB_CSP_STEREO		0x02

#define SNDRV_SB_CSP_RATE_8000		0x01
#define SNDRV_SB_CSP_RATE_11025		0x02
#define SNDRV_SB_CSP_RATE_22050		0x04
#define SNDRV_SB_CSP_RATE_44100		0x08
#define SNDRV_SB_CSP_RATE_ALL		0x0f

#define SNDRV_SB_CSP_ST_IDLE		0x00
#define SNDRV_SB_CSP_ST_LOADED		0x01
#define SNDRV_SB_CSP_ST_RUNNING		0x02
#define SNDRV_SB_CSP_ST_PAUSED		0x04
#define SNDRV_SB_CSP_ST_AUTO		0x08
#define SNDRV_SB_CSP_ST_QSOUND		0x10

#define SNDRV_SB_CSP_QSOUND_MAX_RIGHT	0x20

#define SNDRV_SB_CSP_MAX_MICROCODE_FILE_SIZE	0x3000

struct snd_sb_csp_mc_header {
	char codec_name[16];		
	unsigned short func_req;	
};

struct snd_sb_csp_microcode {
	struct snd_sb_csp_mc_header info;
	unsigned char data[SNDRV_SB_CSP_MAX_MICROCODE_FILE_SIZE];
};

struct snd_sb_csp_start {
	int sample_width;	
	int channels;		
};

struct snd_sb_csp_info {
	char codec_name[16];		
	unsigned short func_nr;		
	unsigned int acc_format;	
	unsigned short acc_channels;	
	unsigned short acc_width;	
	unsigned short acc_rates;	
	unsigned short csp_mode;	
	unsigned short run_channels;	
	unsigned short run_width;	
	unsigned short version;		
	unsigned short state;		
};

#define SNDRV_SB_CSP_IOCTL_INFO		_IOR('H', 0x10, struct snd_sb_csp_info)
#define SNDRV_SB_CSP_IOCTL_LOAD_CODE	\
	_IOC(_IOC_WRITE, 'H', 0x11, sizeof(struct snd_sb_csp_microcode))
#define SNDRV_SB_CSP_IOCTL_UNLOAD_CODE	_IO('H', 0x12)
#define SNDRV_SB_CSP_IOCTL_START		_IOW('H', 0x13, struct snd_sb_csp_start)
#define SNDRV_SB_CSP_IOCTL_STOP		_IO('H', 0x14)
#define SNDRV_SB_CSP_IOCTL_PAUSE		_IO('H', 0x15)
#define SNDRV_SB_CSP_IOCTL_RESTART	_IO('H', 0x16)


#endif 
