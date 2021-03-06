/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
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

#include <stdint.h>
#include <iic_dvfs.h>
#include "board.h"


/************************************************************************
 * Defines
 ************************************************************************/
#ifndef BOARD_DEFAULT
#define BOARD_DEFAULT		(BOARD_SALVATOR_X << BOARD_CODE_SHIFT)
#endif

#define SLAVE_ADDR_EEPROM	(0x50U)
#define	REG_ADDR_BOARD_ID	(0x70U)

#define BOARD_CODE_MASK		(0xF8U)
#define BOARD_REV_MASK		(0x07U)
#define BOARD_CODE_SHIFT	(3U)

#define BOARD_ID_UNKNOWN	(0xFFU)


/************************************************************************
 * Global variables
 ************************************************************************/
const char *g_board_tbl[] = {
	[BOARD_SALVATOR_X]	= "Salvator-X",
	[BOARD_KRIEK]		= "Kriek",
	[BOARD_STARTER_KIT]	= "Starter Kit",
	[BOARD_SALVATOR_XS]	= "Salvator-XS",
	[BOARD_STARTER_KIT_PRE]	= "Starter Kit",
	[BOARD_UNKNOWN]		= "unknown"
};


int32_t get_board_type(uint32_t *type, uint32_t *rev)
{
	int32_t ret = 0;
	uint8_t read_rev;
	static uint8_t g_board_id = BOARD_ID_UNKNOWN;
	const uint8_t board_tbl[][8U] = {
		[BOARD_SALVATOR_X]	= {0x10U, 0x11U, 0xFFU, 0xFFU,
					   0xFFU, 0xFFU, 0xFFU, 0xFFU},
		[BOARD_KRIEK]		= {0x10U, 0xFFU, 0xFFU, 0xFFU,
					   0xFFU, 0xFFU, 0xFFU, 0xFFU},
		[BOARD_STARTER_KIT]	= {0x10U, 0xFFU, 0xFFU, 0xFFU,
					   0xFFU, 0xFFU, 0xFFU, 0xFFU},
		[BOARD_SALVATOR_XS]	= {0x10U, 0xFFU, 0xFFU, 0xFFU,
					   0xFFU, 0xFFU, 0xFFU, 0xFFU},
		[BOARD_STARTER_KIT_PRE]	= {0x10U, 0x10U, 0xFFU, 0xFFU,
					   0xFFU, 0xFFU, 0xFFU, 0xFFU},
	};

	if (BOARD_ID_UNKNOWN == g_board_id) {
#if PMIC_ROHM_BD9571
		/* Board ID detection from EEPROM */
		ret = rcar_iic_dvfs_recieve(SLAVE_ADDR_EEPROM,
			REG_ADDR_BOARD_ID, &g_board_id);
		if (0 != ret) {
			g_board_id = BOARD_ID_UNKNOWN;
		} else if (BOARD_ID_UNKNOWN == g_board_id) {
			/* Can't recognize the board */
			g_board_id = BOARD_DEFAULT;
		} else {
			/* none */
		}
#else	/* PMIC_ROHM_BD9571 */
		g_board_id = BOARD_DEFAULT;
#endif	/* PMIC_ROHM_BD9571 */
	}

	*type = ((uint32_t)g_board_id & BOARD_CODE_MASK) >> BOARD_CODE_SHIFT;
	if (*type < (sizeof(board_tbl) / sizeof(board_tbl[0]))) {
		read_rev = (uint8_t)(g_board_id & BOARD_REV_MASK);
		*rev = board_tbl[*type][read_rev];
	} else {
		/* If there is no revision information, set Rev0.0. */
		*rev = 0x00U;
	}

	return ret;
}
