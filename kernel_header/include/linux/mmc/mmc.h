/*
 * Header for MultiMediaCard (MMC)
 *
 * Copyright 2002 Hewlett-Packard Company
 *
 * Use consistent with the GNU GPL is permitted,
 * provided that this copyright notice is
 * preserved in its entirety in all copies and derived works.
 *
 * HEWLETT-PACKARD COMPANY MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
 * AS TO THE USEFULNESS OR CORRECTNESS OF THIS CODE OR ITS
 * FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 * Many thanks to Alessandro Rubini and Jonathan Corbet!
 *
 * Based strongly on code by:
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 *
 * Author:  Andrew Christian
 *          15 May 2002
 */

#ifndef LINUX_MMC_MMC_H
#define LINUX_MMC_MMC_H

   
#define MMC_GO_IDLE_STATE         0   
#define MMC_SEND_OP_COND          1   
#define MMC_ALL_SEND_CID          2   
#define MMC_SET_RELATIVE_ADDR     3   
#define MMC_SET_DSR               4   
#define MMC_SLEEP_AWAKE		  5   
#define MMC_SWITCH                6   
#define MMC_SELECT_CARD           7   
#define MMC_SEND_EXT_CSD          8   
#define MMC_SEND_CSD              9   
#define MMC_SEND_CID             10   
#define MMC_READ_DAT_UNTIL_STOP  11   
#define MMC_STOP_TRANSMISSION    12   
#define MMC_SEND_STATUS          13   
#define MMC_BUS_TEST_R           14   
#define MMC_GO_INACTIVE_STATE    15   
#define MMC_BUS_TEST_W           19   
#define MMC_SPI_READ_OCR         58   
#define MMC_SPI_CRC_ON_OFF       59   

  
#define MMC_SET_BLOCKLEN         16   
#define MMC_READ_SINGLE_BLOCK    17   
#define MMC_READ_MULTIPLE_BLOCK  18   
#define MMC_SEND_TUNING_BLOCK    19   
#define MMC_SEND_TUNING_BLOCK_HS200	21	
#define MMC_SEND_TUNING_BLOCK_HS400	MMC_SEND_TUNING_BLOCK_HS200

  
#define MMC_WRITE_DAT_UNTIL_STOP 20   

  
#define MMC_SET_BLOCK_COUNT      23   
#define MMC_WRITE_BLOCK          24   
#define MMC_WRITE_MULTIPLE_BLOCK 25   
#define MMC_PROGRAM_CID          26   
#define MMC_PROGRAM_CSD          27   

  
#define MMC_SET_WRITE_PROT       28   
#define MMC_CLR_WRITE_PROT       29   
#define MMC_SEND_WRITE_PROT      30   

  
#define MMC_ERASE_GROUP_START    35   
#define MMC_ERASE_GROUP_END      36   
#define MMC_ERASE                38   

  
#define MMC_FAST_IO              39   
#define MMC_GO_IRQ_STATE         40   

  
#define MMC_LOCK_UNLOCK          42   

  
#define MMC_APP_CMD              55   
#define MMC_GEN_CMD              56   

#endif 
