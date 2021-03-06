/*
 *  linux/drivers/char/serial_core.h
 *
 *  Copyright (C) 2000 Deep Blue Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef LINUX_SERIAL_CORE_H
#define LINUX_SERIAL_CORE_H

#include <linux/serial.h>

#define PORT_UNKNOWN	0
#define PORT_8250	1
#define PORT_16450	2
#define PORT_16550	3
#define PORT_16550A	4
#define PORT_CIRRUS	5
#define PORT_16650	6
#define PORT_16650V2	7
#define PORT_16750	8
#define PORT_STARTECH	9
#define PORT_16C950	10
#define PORT_16654	11
#define PORT_16850	12
#define PORT_RSA	13
#define PORT_NS16550A	14
#define PORT_XSCALE	15
#define PORT_RM9000	16	
#define PORT_OCTEON	17	
#define PORT_AR7	18	
#define PORT_U6_16550A	19	
#define PORT_TEGRA	20	
#define PORT_XR17D15X	21	
#define PORT_MAX_8250	21	

#define PORT_PXA	31
#define PORT_AMBA	32
#define PORT_CLPS711X	33
#define PORT_SA1100	34
#define PORT_UART00	35
#define PORT_21285	37

#define PORT_SUNZILOG	38
#define PORT_SUNSAB	39

#define PORT_DZ		46
#define PORT_ZS		47

#define PORT_MUX	48

#define PORT_ATMEL	49

#define PORT_MAC_ZILOG	50	
#define PORT_PMAC_ZILOG	51

#define PORT_SCI	52
#define PORT_SCIF	53
#define PORT_IRDA	54

#define PORT_S3C2410    55

#define PORT_IP22ZILOG	56

#define PORT_LH7A40X	57

#define PORT_CPM        58

#define PORT_MPC52xx	59

#define PORT_ICOM	60

#define PORT_S3C2440	61

#define PORT_IMX	62

#define PORT_MPSC	63

#define PORT_TXX9	64

#define PORT_VR41XX_SIU		65
#define PORT_VR41XX_DSIU	66

#define PORT_S3C2400	67

#define PORT_M32R_SIO	68

#define PORT_JSM        69

#define PORT_PNX8XXX	70

#define PORT_NETX	71

#define PORT_SUNHV	72

#define PORT_S3C2412	73

#define PORT_UARTLITE	74

#define PORT_BFIN	75

#define PORT_KS8695	76

#define PORT_SB1250_DUART	77

#define PORT_MCF	78

#define PORT_BFIN_SPORT		79

#define PORT_MN10300		80
#define PORT_MN10300_CTS	81

#define PORT_SC26XX	82

#define PORT_SCIFA	83

#define PORT_S3C6400	84

#define PORT_NWPSERIAL	85

#define PORT_MAX3100    86

#define PORT_TIMBUART	87

#define PORT_MSM	88

#define PORT_BCM63XX	89

#define PORT_APBUART    90

#define PORT_ALTERA_JTAGUART	91
#define PORT_ALTERA_UART	92

#define PORT_SCIFB	93

#define PORT_MAX3107	94

#define PORT_MFD	95

#define PORT_OMAP	96

#define PORT_VT8500	97

#define PORT_XUARTPS	98

#define PORT_AR933X	99

#define PORT_EFMUART   100


#endif 
