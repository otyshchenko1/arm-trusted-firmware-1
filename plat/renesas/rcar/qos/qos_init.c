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

#include <stdint.h>
#include <debug.h>
#include <mmio.h>
#include "qos_init.h"
#include "qos_common.h"
#if RCAR_LSI == RCAR_AUTO
  #include "H3/qos_init_h3_v10.h"
  #include "H3/qos_init_h3_v11.h"
  #include "H3/qos_init_h3_v20.h"
  #include "M3/qos_init_m3_v10.h"
  #include "M3/qos_init_m3_v11.h"
  #include "M3N/qos_init_m3n_v10.h"
#endif
#if RCAR_LSI == RCAR_H3	/* H3 */
  #include "H3/qos_init_h3_v10.h"
  #include "H3/qos_init_h3_v11.h"
  #include "H3/qos_init_h3_v20.h"
#endif
#if RCAR_LSI == RCAR_M3	/* M3 */
  #include "M3/qos_init_m3_v10.h"
  #include "M3/qos_init_m3_v11.h"
#endif
#if RCAR_LSI == RCAR_M3N	/* M3N */
  #include "M3N/qos_init_m3n_v10.h"
#endif

 /* Product Register */
#define PRR			(0xFFF00044U)
#define PRR_PRODUCT_MASK	(0x00007F00U)
#define PRR_CUT_MASK		(0x000000FFU)
#define PRR_PRODUCT_H3		(0x00004F00U)           /* R-Car H3 */
#define PRR_PRODUCT_M3		(0x00005200U)           /* R-Car M3 */
#define PRR_PRODUCT_M3N		(0x00005500U)           /* R-Car M3N */
#define PRR_PRODUCT_10		(0x00U)
#define PRR_PRODUCT_11		(0x01U)
#define PRR_PRODUCT_20		(0x10U)

#define PRR_PRODUCT_ERR(reg)	do{\
				ERROR("LSI Product ID(PRR=0x%x) QoS "\
				"initialize not supported.\n",reg);\
				panic();\
				}while(0)
#define PRR_CUT_ERR(reg)	do{\
				ERROR("LSI Cut ID(PRR=0x%x) QoS "\
				"initialize not supported.\n",reg);\
				panic();\
				}while(0)

void qos_init(void)
{
	uint32_t reg;

	reg = mmio_read_32(PRR);
#if (RCAR_LSI == RCAR_AUTO) || RCAR_LSI_CUT_COMPAT
	switch (reg & PRR_PRODUCT_MASK) {
	case PRR_PRODUCT_H3:
 #if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_H3)
		switch (reg & PRR_CUT_MASK) {
		case PRR_PRODUCT_10:
			qos_init_h3_v10();
			break;
		case PRR_PRODUCT_11:
			qos_init_h3_v11();
			break;
		case PRR_PRODUCT_20:
		default:
			qos_init_h3_v20();
			break;
		}
 #else
		PRR_PRODUCT_ERR(reg);
 #endif
		break;
	case PRR_PRODUCT_M3:
 #if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_M3)
		switch (reg & PRR_CUT_MASK) {
		case PRR_PRODUCT_10:
			qos_init_m3_v10();
			break;
		case PRR_PRODUCT_20: /* M3 Cut 11 */
		default:
			qos_init_m3_v11();
			break;
		}
 #else
		PRR_PRODUCT_ERR(reg);
 #endif
		break;
	case PRR_PRODUCT_M3N:
 #if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_M3N)
		switch (reg & PRR_CUT_MASK) {
		case PRR_PRODUCT_10:
		default:
			qos_init_m3n_v10();
			break;
		}
 #else
		PRR_PRODUCT_ERR(reg);
 #endif
		break;
	default:
		PRR_PRODUCT_ERR(reg);
		break;
	}
#else
 #if RCAR_LSI == RCAR_H3	/* H3 */
  #if RCAR_LSI_CUT == RCAR_CUT_10
	/* H3 Cut 10 */
	if ((PRR_PRODUCT_H3 | PRR_PRODUCT_10)
			!= (reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_h3_v10();
  #elif RCAR_LSI_CUT == RCAR_CUT_11
	/* H3 Cut 11 */
	if ((PRR_PRODUCT_H3 | PRR_PRODUCT_11)
			!= (reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_h3_v11();
  #else
	/* H3 Cut 20 or later */
	if ((PRR_PRODUCT_H3)
			!= (reg & (PRR_PRODUCT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_h3_v20();
  #endif
 #elif RCAR_LSI == RCAR_M3	/* M3 */
  #if RCAR_LSI_CUT == RCAR_CUT_10
	/* M3 Cut 10 */
	if ((PRR_PRODUCT_M3 | PRR_PRODUCT_10)
			!= (reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_m3_v10();
  #else
	/* M3 Cut 11 or later */
	if ((PRR_PRODUCT_M3)
			!= (reg & (PRR_PRODUCT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_m3_v11();
  #endif
 #elif RCAR_LSI == RCAR_M3N	/* M3N */
	/* M3N Cut 10 or later */
	if ((PRR_PRODUCT_M3N)
			!= (reg & (PRR_PRODUCT_MASK))) {
		PRR_PRODUCT_ERR(reg);
	}
	qos_init_m3n_v10();
 #else
  #error "Don't have QoS initialize routine(Unknown chip)."
 #endif
#endif
}

uint32_t get_refperiod(void)
{
	uint32_t refperiod = QOSWT_WTSET0_CYCLE;

#if (RCAR_LSI == RCAR_AUTO) || RCAR_LSI_CUT_COMPAT
	uint32_t reg;

	reg = mmio_read_32(PRR);
	switch (reg & PRR_PRODUCT_MASK) {
 #if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_M3N)
	case PRR_PRODUCT_M3N:
		refperiod = QOSWT_WTSET0_CYCLE_M3N;
		break;
 #endif
	default:
		break;
	}
#elif RCAR_LSI == RCAR_M3N	/* for M3N */
	refperiod = QOSWT_WTSET0_CYCLE_M3N;
#endif

	return refperiod;
}
