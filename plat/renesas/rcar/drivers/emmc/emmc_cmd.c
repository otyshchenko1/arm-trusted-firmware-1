/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *   - Neither the name of Renesas nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file  emmc_cmd.c
 * @brief MMC card send command operation.
 *
 */

/* ************************ HEADER (INCLUDE) SECTION *********************** */

#include <debug.h>
#include "emmc_config.h"
#include "emmc_hal.h"
#include "emmc_std.h"
#include "emmc_registers.h"
#include "emmc_def.h"
#include "micro_wait.h"
/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */

/* ************************** FUNCTION PROTOTYPES ************************** */
static void emmc_read_response(uint32_t *response);
static void emmc_little_to_big(uint8_t *p, uint32_t value);
static void emmc_data_transfer_dma (void);
static EMMC_ERROR_CODE emmc_response_check(uint32_t *response, uint32_t error_mask);
static void emmc_softreset(void);
static void emmc_WaitCmd2Cmd_8Cycle(void);

static void cmdErrSdInfo2Log(void);

/* ********************************* CODE ********************************** */

/** @brief execute MMC command.
 *
 * - Pre-conditions:<BR>
 *  * Clock to memory card IF is enabled.
 * - Post-conditions:<BR>
 * Requested command is executed successfully
 *
 * @param[in] error_mask Errors to be checked (error values; HAL_MEMCARD_ERRORS)
 * @param[in,out] *response Response from the card  (virtual address)
 * @return eMMC error code.
 */
EMMC_ERROR_CODE emmc_exec_cmd (
    uint32_t error_mask,
    uint32_t *response        /**<  virtual address */
    )
{
    EMMC_ERROR_CODE rtn_code = EMMC_SUCCESS;
    HAL_MEMCARD_RESPONSE_TYPE response_type;
    HAL_MEMCARD_COMMAND_TYPE cmd_type;
    EMMC_INT_STATE state;
    uint32_t	err_not_care_flag = FALSE;

    /* parameter check */
    if (response == NULL)
    {
        emmc_write_error_info(EMMC_FUNCNO_EXEC_CMD, EMMC_ERR_PARAM);
        return EMMC_ERR_PARAM;
    }

    /* state check */
    if (mmc_drv_obj.clock_enable != TRUE)
    {
        emmc_write_error_info(EMMC_FUNCNO_EXEC_CMD, EMMC_ERR_STATE);
        return EMMC_ERR_STATE;
    }

    if (mmc_drv_obj.state_machine_blocking == TRUE)
    {
        emmc_write_error_info(EMMC_FUNCNO_EXEC_CMD, EMMC_ERR);
        return EMMC_ERR;
    }

    state = ESTATE_BEGIN;
    response_type = (HAL_MEMCARD_RESPONSE_TYPE)(mmc_drv_obj.cmd_info.cmd&HAL_MEMCARD_RESPONSE_TYPE_MASK);
    cmd_type = (HAL_MEMCARD_COMMAND_TYPE)(mmc_drv_obj.cmd_info.cmd&HAL_MEMCARD_COMMAND_TYPE_MASK);

	/* state machine */
    while ( (mmc_drv_obj.force_terminate != TRUE) && (state != ESTATE_END) )
    {
        /* The interrupt factor flag is observed. */
    	emmc_interrupt();

        /* wait interrupt */
        if (mmc_drv_obj.state_machine_blocking == TRUE)
        {
            continue;
        }

    	switch (state)
        {
            case ESTATE_BEGIN:
                /* Busy check */
				if( ( mmc_drv_obj.error_info.info2 & SD_INFO2_CBSY ) != 0 )
                {
                    emmc_write_error_info(EMMC_FUNCNO_EXEC_CMD, EMMC_ERR_CARD_BUSY);
                    return EMMC_ERR_CARD_BUSY;
                }

				/* clear register */
                SETR_32( SD_INFO1, 0x00000000U );
                SETR_32( SD_INFO2, SD_INFO2_CLEAR );
                SETR_32( SD_INFO1_MASK, SD_INFO1_INFO0 );
                SETR_32( SD_INFO2_MASK, ( SD_INFO2_ALL_ERR | SD_INFO2_CLEAR ) );

                state = ESTATE_ISSUE_CMD;
                /* through */

            case ESTATE_ISSUE_CMD:
                /* ARG */
                SETR_32( SD_ARG, mmc_drv_obj.cmd_info.arg );
                /* issue cmd */
                SETR_32(SD_CMD, mmc_drv_obj.cmd_info.hw);
                /* Set driver flag */
                mmc_drv_obj.during_cmd_processing = TRUE;
                mmc_drv_obj.state_machine_blocking = TRUE;

                if (response_type == HAL_MEMCARD_RESPONSE_NONE)
                {
                    state = ESTATE_NON_RESP_CMD;
                }
                else
                {
                    state = ESTATE_RCV_RESP;
                }

                break;

            case ESTATE_NON_RESP_CMD:
                /* interrupt disable */
                SETR_32(SD_INFO1_MASK, 0x00000000U);
                SETR_32(SD_INFO2_MASK, SD_INFO2_CLEAR );

                /* check interrupt */
                if ((mmc_drv_obj.int_event2 & SD_INFO2_ALL_ERR) != 0)
                {
                    /* error interrupt */
                	cmdErrSdInfo2Log();
                    rtn_code = EMMC_ERR_INFO2;
                    state = ESTATE_ERROR;
                }
                else if((mmc_drv_obj.int_event1 & SD_INFO1_INFO0) == 0)
                {
                    /* not receive expected interrupt */
                    rtn_code = EMMC_ERR_RESPONSE;
                    state = ESTATE_ERROR;
                }
                else
                {
                    emmc_WaitCmd2Cmd_8Cycle();
                    state = ESTATE_END;
                }
                break;

            case ESTATE_RCV_RESP:
                /* interrupt disable */
                SETR_32(SD_INFO1_MASK, 0x00000000U);
                SETR_32(SD_INFO2_MASK, SD_INFO2_CLEAR );

                /* check interrupt */
                if((mmc_drv_obj.int_event2 & SD_INFO2_ALL_ERR) != 0)
                {
			if ((mmc_drv_obj.get_partition_access_flag == TRUE) &&
				((mmc_drv_obj.int_event2 & SD_INFO2_ERR6) != 0U)) {
				err_not_care_flag = TRUE;
				rtn_code = EMMC_ERR_CMD_TIMEOUT;
			} else {
				/* error interrupt */
				cmdErrSdInfo2Log();
				rtn_code = EMMC_ERR_INFO2;
			}
			state = ESTATE_ERROR;
                    break;
                }
                else if((mmc_drv_obj.int_event1 & SD_INFO1_INFO0) == 0)
                {
                    /* not receive expected interrupt */
                    rtn_code = EMMC_ERR_RESPONSE;
                    state = ESTATE_ERROR;
                    break;
                }

                /* read response */
                emmc_read_response(response);

				/* check response */
                rtn_code = emmc_response_check(response, error_mask);
                if (rtn_code != EMMC_SUCCESS)
                {
                    state = ESTATE_ERROR;
                    break;
                }

                if (response_type == HAL_MEMCARD_RESPONSE_R1b)
                {
                    /* R1b */
	                SETR_32( SD_INFO2_MASK, ( SD_INFO2_ALL_ERR | SD_INFO2_CLEAR ) );
                    state = ESTATE_RCV_RESPONSE_BUSY;
                }
                else
                {
                    state = ESTATE_CHECK_RESPONSE_COMPLETE;
                }
                break;

            case ESTATE_RCV_RESPONSE_BUSY:
                /* check interrupt */
                if ((mmc_drv_obj.int_event2 & SD_INFO2_ALL_ERR) != 0)
                {
                    /* error interrupt */
                	cmdErrSdInfo2Log();
                    rtn_code = EMMC_ERR_INFO2;
                    state = ESTATE_ERROR;
                    break;
                }
        		/* DAT0 not Busy */
                if( (SD_INFO2_DAT0 & mmc_drv_obj.error_info.info2) != 0 )
			    {
                	state = ESTATE_CHECK_RESPONSE_COMPLETE;
			    	break;
			    }
            	break;

            case ESTATE_CHECK_RESPONSE_COMPLETE:
                if (cmd_type >= HAL_MEMCARD_COMMAND_TYPE_ADTC_WRITE)
                {
                    state = ESTATE_DATA_TRANSFER;
                }
                else
                {
					emmc_WaitCmd2Cmd_8Cycle();
                	state = ESTATE_END;
                }
                break;

            case ESTATE_DATA_TRANSFER:
                /* ADTC command  */
                mmc_drv_obj.during_transfer = TRUE;
                mmc_drv_obj.state_machine_blocking = TRUE;

                if (mmc_drv_obj.transfer_mode == HAL_MEMCARD_DMA)
                {
                   /* DMA */
                    emmc_data_transfer_dma();
                }
                else
                {
                    /* PIO */
                    /* interrupt enable (FIFO read/write enable) */
                    if (mmc_drv_obj.cmd_info.dir == HAL_MEMCARD_WRITE)
                    {
                        SETR_32(SD_INFO2_MASK, ( SD_INFO2_BWE | SD_INFO2_ALL_ERR | SD_INFO2_CLEAR ) );
                    }
                    else
                    {
                        SETR_32(SD_INFO2_MASK, ( SD_INFO2_BRE | SD_INFO2_ALL_ERR | SD_INFO2_CLEAR ) );
                    }
                }
                state = ESTATE_DATA_TRANSFER_COMPLETE;
                break;

            case  ESTATE_DATA_TRANSFER_COMPLETE:
				/* check interrupt */
                if( ( mmc_drv_obj.int_event2 & SD_INFO2_ALL_ERR ) != 0)
                {
                    /* error interrupt */
                	cmdErrSdInfo2Log();
                    rtn_code = EMMC_ERR_INFO2;
                    state = ESTATE_TRANSFER_ERROR;
                    break;
                }
				else
                {
                    /* success. nothing to do. */
                }

                /* DMAC error ? */
                if (mmc_drv_obj.dma_error_flag == TRUE)
                {
                    /* Error occurred in DMAC driver. */
                    rtn_code = EMMC_ERR_FROM_DMAC_TRANSFER;
                    state = ESTATE_TRANSFER_ERROR;
                }
                else if (mmc_drv_obj.during_dma_transfer == TRUE)
                {
                    /* DMAC not finished. unknown error */
                    rtn_code = EMMC_ERR;
                    state = ESTATE_TRANSFER_ERROR;
                }
    			else
                {
	                SETR_32( SD_INFO1_MASK, SD_INFO1_INFO2 );
                    SETR_32( SD_INFO2_MASK, ( SD_INFO2_ALL_ERR | SD_INFO2_CLEAR ) );

	                mmc_drv_obj.state_machine_blocking = TRUE;

                    state = ESTATE_ACCESS_END;
                }
                break;

            case ESTATE_ACCESS_END:

				/* clear flag */
                if(HAL_MEMCARD_DMA == mmc_drv_obj.transfer_mode)
        		{
	        		SETR_32( CC_EXT_MODE, CC_EXT_MODE_CLEAR );			/* W (CC_EXT_MODE, H'0000_1010) SD_BUF DMA transfer disabled */
		            SETR_32( SD_STOP, 0x00000000U );
       			    mmc_drv_obj.during_dma_transfer = FALSE;
        		}

	            SETR_32( SD_INFO1_MASK, 0x00000000U );
	            SETR_32( SD_INFO2_MASK, SD_INFO2_CLEAR );
	            SETR_32( SD_INFO1, 0x00000000U );
	            SETR_32( SD_INFO2, SD_INFO2_CLEAR );

        		if(( mmc_drv_obj.int_event1 & SD_INFO1_INFO2 ) != 0 )
        		{
                    emmc_WaitCmd2Cmd_8Cycle();
                    state = ESTATE_END;
        		}
        		else
        		{
        			state = ESTATE_ERROR;
        		}
        		break;

            case ESTATE_TRANSFER_ERROR:
                /* The error occurred in the Data transfer.  */
                if(HAL_MEMCARD_DMA == mmc_drv_obj.transfer_mode)
                {
	        		SETR_32( CC_EXT_MODE, CC_EXT_MODE_CLEAR );			/* W (CC_EXT_MODE, H'0000_1010) SD_BUF DMA transfer disabled */
		            SETR_32( SD_STOP, 0x00000000U );
       			    mmc_drv_obj.during_dma_transfer = FALSE;
                }
                /* through */

            case ESTATE_ERROR:
        	if (err_not_care_flag == TRUE) {
			mmc_drv_obj.during_cmd_processing = FALSE;
        	} else {
			emmc_softreset();
			emmc_write_error_info(EMMC_FUNCNO_EXEC_CMD, rtn_code);
        	}
                return rtn_code;

            default:
                state = ESTATE_END;
                break;
        }   /* switch (state) */
    }   /*  while ( (mmc_drv_obj.force_terminate != TRUE) && (state != ESTATE_END) ) */

    /* force terminate */
    if (mmc_drv_obj.force_terminate == TRUE)
    {
        /* timeout timer is expired. Or, PIO data transfer error. */
        /* Timeout occurred in the DMA transfer. */
        if (mmc_drv_obj.during_dma_transfer == TRUE)
        {
            mmc_drv_obj.during_dma_transfer = FALSE;
        }
        ERROR("BL2: emmc exec_cmd:EMMC_ERR_FORCE_TERMINATE\n");
        emmc_softreset();

        return EMMC_ERR_FORCE_TERMINATE;    /* error information has already been written. */
    }

    /* success */
    mmc_drv_obj.during_cmd_processing = FALSE;
    mmc_drv_obj.during_transfer = FALSE;

    return EMMC_SUCCESS;
}

/** @brief host controller softreset.
 *
 * - Pre-conditions:<BR>
 * .
 * - Post-conditions:<BR>
 * .
 *
 * @param None.
 * @return None.
 */
static void emmc_softreset(void)
{
    int32_t     loop = 10000;
    int32_t     retry = 1000;

    /* flag clear */
    mmc_drv_obj.during_cmd_processing   = FALSE;
    mmc_drv_obj.during_transfer         = FALSE;
    mmc_drv_obj.during_dma_transfer     = FALSE;
    mmc_drv_obj.state_machine_blocking  = FALSE;
    mmc_drv_obj.force_terminate         = FALSE;
    mmc_drv_obj.dma_error_flag          = FALSE;

    /* during operation ? */
    if ((GETR_32(SD_INFO2) & SD_INFO2_CBSY) != 0)
    {
        /* wait CMDSEQ = 0 */
        while (loop > 0)
        {
            if((GETR_32(SD_INFO2) & SD_INFO2_CBSY) == 0)
            {
                break;  /* ready */
            }

            loop--;
            if ((loop == 0) && (retry > 0) )
            {
		micro_wait(1000U);		/* wait 1ms */
                loop = 10000;
                retry--;
            }
        }
    }

    /* reset */
	SETR_32(SOFT_RST, ( GETR_32(SOFT_RST) & (~SOFT_RST_SDRST) ) );		/* Soft reset */
	SETR_32(SOFT_RST, ( GETR_32(SOFT_RST) | SOFT_RST_SDRST ) );		/* Soft reset released */

    /* initialize */
    SETR_32(SD_INFO1, 0x00000000U);
    SETR_32(SD_INFO2, SD_INFO2_CLEAR );
    SETR_32(SD_INFO1_MASK, 0x00000000U);      /* all interrupt disable */
    SETR_32(SD_INFO2_MASK, SD_INFO2_CLEAR );  /* all interrupt disable */

}

/** @brief read response
 *
 * - Pre-conditions:<BR>
 * Called from emmc_exec_cmd().
 * - Post-conditions:<BR>
 * .
 *
 * @param[in,out] *response Response from the card
 * @return None.
 */
static void emmc_read_response(
    uint32_t *response
    )
{

    uint8_t   *p;

    if (response == NULL)
    {
        return;
    }

    /* read response */
    if (mmc_drv_obj.response_length == EMMC_MAX_RESPONSE_LENGTH)
    {
        /* CSD or CID */
        p = (uint8_t *)(response);
        emmc_little_to_big(p	, ((GETR_32(SD_RSP76)<<8)|(GETR_32(SD_RSP54)>>24)));	/* [127:96]	*/
        emmc_little_to_big(p+4	, ((GETR_32(SD_RSP54)<<8)|(GETR_32(SD_RSP32)>>24)));	/* [95:64]	*/
        emmc_little_to_big(p+8	, ((GETR_32(SD_RSP32)<<8)|(GETR_32(SD_RSP10)>>24)));	/* [63:32]	*/
        emmc_little_to_big(p+12	, (GETR_32(SD_RSP10)<<8));								/* [31:0]	*/
    }
    else
    {
        *response = GETR_32(SD_RSP10);  /* [39:8] */
    }
}

/** @brief response check
 *
 * - Pre-conditions:<BR>
 * Called from emmc_exec_cmd().
 * - Post-conditions:<BR>
 * .
 *
 * @param[in] *response Response from the card
 * @param[in] error_mask  Errors to be checked (for R1/R1b response)
 * @return error code.
 */
static EMMC_ERROR_CODE emmc_response_check(
    uint32_t *response,
    uint32_t error_mask
    )
{

    HAL_MEMCARD_RESPONSE_TYPE response_type = (HAL_MEMCARD_RESPONSE_TYPE)(mmc_drv_obj.cmd_info.cmd&HAL_MEMCARD_RESPONSE_TYPE_MASK);

    if (response == NULL)
    {
        return EMMC_ERR_PARAM;
    }

    if (response_type == HAL_MEMCARD_RESPONSE_NONE)
    {
        return EMMC_SUCCESS;
    }

    /* response check */
    if (response_type <= HAL_MEMCARD_RESPONSE_R1b)
    {
        /* R1 or R1b */
        mmc_drv_obj.current_state = (EMMC_R1_STATE)((*response & EMMC_R1_STATE_MASK) >> EMMC_R1_STATE_SHIFT);
        if ( (*response & error_mask) != 0)
        {
        	if((0x80 & *response) != 0)
        	{
				ERROR("BL2: emmc SWITCH_ERROR\n");
        	}
            return EMMC_ERR_CARD_STATUS_BIT;
        }
    }
    else if (response_type == HAL_MEMCARD_RESPONSE_R4)
    {
        /* R4 */
        if ( (*response & EMMC_R4_STATUS) != 0)
        {
            return EMMC_ERR_CARD_STATUS_BIT;
        }
    }
    else
    {
        /* nothing to do. other type does not have status bit */
    }

    return EMMC_SUCCESS;
}

/** @brief converts endian from little to big
 *
 * - Pre-conditions:<BR>
 * .
 * - Post-conditions:<BR>
 * .
 *
 * @param[in,out] p destination buffer address.
 * @param[in] value convert data.(little)
 * @return None.
 */
static void emmc_little_to_big(
    uint8_t *p,
    uint32_t value
    )
{
    if (p == NULL)
    {
        return;
    }

    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

/** @brief data transfer with DMA.
 *
 * - Pre-conditions:<BR>
 * Called from emmc_exec_cmd().
 * - Post-conditions:<BR>
 * .
 *
 * @return error code.
 */
static void emmc_data_transfer_dma(void)
{
	mmc_drv_obj.during_dma_transfer = TRUE;
    mmc_drv_obj.dma_error_flag = FALSE;

    SETR_32( SD_INFO1_MASK, 0x00000000U );
    SETR_32( SD_INFO2_MASK, ( SD_INFO2_ALL_ERR | SD_INFO2_CLEAR ) );

	/* DMAC setting */
    if (mmc_drv_obj.cmd_info.dir == HAL_MEMCARD_WRITE)
    {
        /* transfer complete interrupt enable */
        SETR_32( DM_CM_INFO1_MASK, ( DM_CM_INFO_MASK_CLEAR | DM_CM_INFO_CH0_ENABLE ) );
        SETR_32( DM_CM_INFO2_MASK, ( DM_CM_INFO_MASK_CLEAR | DM_CM_INFO_CH0_ENABLE ) );
        /* BUFF --> FIFO */
    	SETR_32( DM_CM_DTRAN_MODE, ( DM_CM_DTRAN_MODE_CH0 | DM_CM_DTRAN_MODE_BIT_WIDTH ) );     /* CH0(downstream), 64-bit width */
    }
    else
    {
        /* transfer complete interrupt enable */
        SETR_32( DM_CM_INFO1_MASK, ( DM_CM_INFO_MASK_CLEAR | DM_CM_INFO_CH1_ENABLE ) );
        SETR_32( DM_CM_INFO2_MASK, ( DM_CM_INFO_MASK_CLEAR | DM_CM_INFO_CH1_ENABLE ) );
        /* FIFO --> BUFF */
    	SETR_32( DM_CM_DTRAN_MODE, ( DM_CM_DTRAN_MODE_CH1 | DM_CM_DTRAN_MODE_BIT_WIDTH ) );     /* CH0(downstream), 64-bit width */
    }
	SETR_32( DM_DTRAN_ADDR, ( ( (uintptr_t)mmc_drv_obj.buff_address_virtual & DM_DTRAN_ADDR_WRITE_MASK ) ) );    /* Set address */

	SETR_32( DM_CM_DTRAN_CTRL, DM_CM_DTRAN_CTRL_START );     /* DMAC Start */
}

/** @brief wait cmd-cmd 8cycle
 *
 * - Pre-conditions:<BR>
 *
 * - Post-conditions:<BR>
 * .
 *
 * @return None.
 */
static void emmc_WaitCmd2Cmd_8Cycle(void)
{
	uint32_t dataL,wait=0;

	dataL = GETR_32(SD_CLK_CTRL);
	dataL &= 0x000000FF;

	switch(dataL){
		case 0xFF: wait = 10U;  break;  // 1/1    10 us wait  (  1/200MHz)*8=  0.04 us(min)
		case 0x00: wait = 10U;  break;  // 1/2    10 us wait  (  2/200MHz)*8=  0.08 us(min)
		case 0x01: wait = 10U;  break;  // 1/4    10 us wait  (  4/200MHz)*8=  0.16 us(min)
		case 0x02: wait = 10U;  break;  // 1/8    10 us wait  (  8/200MHz)*8=  0.32 us(min)
		case 0x04: wait = 10U;  break;  // 1/16   10 us wait  ( 16/200MHz)*8=  0.64 us(min)
		case 0x08: wait = 10U;  break;  // 1/32   10 us wait  ( 32/200MHz)*8=  1.28 us(min)
		case 0x10: wait = 10U;  break;  // 1/64   10 us wait  ( 64/200MHz)*8=  2.56 us(min)
		case 0x20: wait = 10U;  break;  // 1/128  10 us wait  (128/200MHz)*8=  5.12 us(min)
		case 0x40: wait = 20U;  break;  // 1/256  20 us wait  (256/200MHz)*8= 10.24 us(min)
		case 0x80: wait = 30U;  break;  // 1/512  30 us wait  (512/200MHz)*8= 20.48 us(min)
	}
	micro_wait(wait);

}

/** @brief debug log SD_INFO2
 *
 * - Pre-conditions:<BR>
 *
 * - Post-conditions:<BR>
 * .
 *
 * @return None.
 */
static void cmdErrSdInfo2Log( void )
{
	ERROR("BL2: emmc ERR SD_INFO2 = 0x%x\n", mmc_drv_obj.error_info.info2);
}
/* ******************************** END ************************************ */

