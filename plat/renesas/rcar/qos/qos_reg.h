/*
 * Copyright (c) 2017, Renesas Electronics Corporation
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

#ifndef QOS_REG_H_
#define QOS_REG_H_

#define	RCAR_QOS_NONE				(3U)
#define	RCAR_QOS_TYPE_DEFAULT		(0U)

#define	RCAR_DRAM_SPLIT_LINEAR		(0U)
#define	RCAR_DRAM_SPLIT_4CH			(1U)
#define	RCAR_DRAM_SPLIT_2CH			(2U)
#define	RCAR_DRAM_SPLIT_AUTO		(3U)
#define	RST_BASE					(0xE6160000U)
#define	RST_MODEMR					(RST_BASE + 0x0060U)

#define	DBSC_BASE					(0xE6790000U)
#define DBSC_DBCAM0CNF1				(DBSC_BASE + 0x0904U)
#define DBSC_DBCAM0CNF2				(DBSC_BASE + 0x0908U)
#define DBSC_DBCAM0CNF3				(DBSC_BASE + 0x090CU)
#define DBSC_DBSCHCNT0				(DBSC_BASE + 0x1000U)
#define DBSC_DBSCHSZ0				(DBSC_BASE + 0x1010U)
#define DBSC_DBSCHRW0				(DBSC_BASE + 0x1020U)
#define DBSC_DBSCHQOS00				(DBSC_BASE + 0x1030U)
#define DBSC_DBSCHQOS01				(DBSC_BASE + 0x1034U)
#define DBSC_DBSCHQOS02				(DBSC_BASE + 0x1038U)
#define DBSC_DBSCHQOS03				(DBSC_BASE + 0x103CU)
#define DBSC_DBSCHQOS40				(DBSC_BASE + 0x1070U)
#define DBSC_DBSCHQOS41				(DBSC_BASE + 0x1074U)
#define DBSC_DBSCHQOS42				(DBSC_BASE + 0x1078U)
#define DBSC_DBSCHQOS43				(DBSC_BASE + 0x107CU)
#define DBSC_DBSCHQOS90				(DBSC_BASE + 0x10C0U)
#define DBSC_DBSCHQOS91				(DBSC_BASE + 0x10C4U)
#define DBSC_DBSCHQOS92				(DBSC_BASE + 0x10C8U)
#define DBSC_DBSCHQOS93				(DBSC_BASE + 0x10CCU)
#define DBSC_DBSCHQOS120			(DBSC_BASE + 0x10F0U)
#define DBSC_DBSCHQOS121			(DBSC_BASE + 0x10F4U)
#define DBSC_DBSCHQOS122			(DBSC_BASE + 0x10F8U)
#define DBSC_DBSCHQOS123			(DBSC_BASE + 0x10FCU)
#define DBSC_DBSCHQOS130			(DBSC_BASE + 0x1100U)
#define DBSC_DBSCHQOS131			(DBSC_BASE + 0x1104U)
#define DBSC_DBSCHQOS132			(DBSC_BASE + 0x1108U)
#define DBSC_DBSCHQOS133			(DBSC_BASE + 0x110CU)
#define DBSC_DBSCHQOS140			(DBSC_BASE + 0x1110U)
#define DBSC_DBSCHQOS141			(DBSC_BASE + 0x1114U)
#define DBSC_DBSCHQOS142			(DBSC_BASE + 0x1118U)
#define DBSC_DBSCHQOS143			(DBSC_BASE + 0x111CU)
#define DBSC_DBSCHQOS150			(DBSC_BASE + 0x1120U)
#define DBSC_DBSCHQOS151			(DBSC_BASE + 0x1124U)
#define DBSC_DBSCHQOS152			(DBSC_BASE + 0x1128U)
#define DBSC_DBSCHQOS153			(DBSC_BASE + 0x112CU)
#define DBSC_SCFCTST2				(DBSC_BASE + 0x170CU)

#define	AXI_BASE					(0xE6784000U)
#define	AXI_ADSPLCR0				(AXI_BASE + 0x0008U)
#define	AXI_ADSPLCR1				(AXI_BASE + 0x000CU)
#define	AXI_ADSPLCR2				(AXI_BASE + 0x0010U)
#define	AXI_ADSPLCR3				(AXI_BASE + 0x0014U)
#define	ADSPLCR0_ADRMODE_DEFAULT	((uint32_t)0U << 31U)
#define	ADSPLCR0_ADRMODE_GEN2		((uint32_t)1U << 31U)
#define	ADSPLCR0_SPLITSEL(x)		((uint32_t)(x) << 16U)
#define	ADSPLCR0_AREA(x)			((uint32_t)(x) <<  8U)
#define	ADSPLCR0_SWP				(0x0CU)

#define	QOS_BASE0					(0xE67E0000U)
#define	QOSBW_FIX_QOS_BANK0			(QOS_BASE0 + 0x0000U)
#define	QOSBW_FIX_QOS_BANK1			(QOS_BASE0 + 0x1000U)
#define	QOSBW_BE_QOS_BANK0			(QOS_BASE0 + 0x2000U)
#define	QOSBW_BE_QOS_BANK1			(QOS_BASE0 + 0x3000U)
#define	QOSCTRL_SL_INIT				(QOS_BASE0 + 0x8000U)
#define	QOSCTRL_REF_ARS				(QOS_BASE0 + 0x8004U)
#define	QOSCTRL_STATQC				(QOS_BASE0 + 0x8008U)

#define	QOS_BASE1					(0xE67F0000U)
#define	QOSCTRL_RAS					(QOS_BASE1 + 0x0000U)
#define	QOSCTRL_RAEN				(QOS_BASE1 + 0x0018U)
#define	QOSCTRL_DANN				(QOS_BASE1 + 0x0030U)
#define	QOSCTRL_DANT				(QOS_BASE1 + 0x0038U)
#define	QOSCTRL_INSFC				(QOS_BASE1 + 0x0050U)
#define	QOSCTRL_RACNT0				(QOS_BASE1 + 0x0080U)
#define	QOSCTRL_STATGEN0			(QOS_BASE1 + 0x0088U)

#define	GPU_ACT_GRD					(0xFD820808U)
#define	GPU_ACT0					(0xFD820800U)
#define	GPU_ACT1					(0xFD821800U)
#define	GPU_ACT2					(0xFD822800U)
#define	GPU_ACT3					(0xFD823800U)
#define	GPU_ACT4					(0xFD824800U)
#define	GPU_ACT5					(0xFD825800U)
#define	GPU_ACT6					(0xFD826800U)
#define	GPU_ACT7					(0xFD827800U)

#define	RT_ACT0						(0xFFC50800U)
#define	RT_ACT1						(0xFFC51800U)

#define	CPU_ACT0					(0xF1300800U)
#define	CPU_ACT1					(0xF1340800U)
#define	CPU_ACT2					(0xF1380800U)
#define	CPU_ACT3					(0xF13C0800U)

#endif /* QOS_REG_H_ */