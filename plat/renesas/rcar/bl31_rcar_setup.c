/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <bl_common.h>
#include <bl31.h>
#include <console.h>
#include <mmio.h>
#include <platform.h>
#include <stddef.h>
#include <debug.h>
#include <plat_arm.h>
#include "drivers/pwrc/rcar_pwrc.h"
#include "rcar_def.h"
#include "rcar_private.h"
#include "rcar_version.h"

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
extern unsigned long __RO_START__;
extern unsigned long __RO_END__;
extern unsigned long __BL31_END__;

#if USE_COHERENT_MEM
extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;
#endif

/*
 * The next 3 constants identify the extents of the code, RO data region and the
 * limit of the BL3-1 image.  These addresses are used by the MMU setup code and
 * therefore they must be page-aligned.  It is the responsibility of the linker
 * script to ensure that __RO_START__, __RO_END__ & __BL31_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_RO_BASE (uint64_t)(&__RO_START__)
#define BL31_RO_LIMIT (uint64_t)(&__RO_END__)
#define BL31_END (uint64_t)(&__BL31_END__)

#if USE_COHERENT_MEM
/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (uint64_t)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (uint64_t)(&__COHERENT_RAM_END__)
#endif

/*******************************************************************************
 * Reference to structure which holds the arguments that have been passed to
 * BL31 from BL2.
 ******************************************************************************/
static bl31_params_t *bl2_to_bl31_params;

uint32_t rcar_boot_kind_flag __attribute__((section("data")));

static uint64_t rcar_boot_mpidr;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info =
			(type == NON_SECURE) ?
					bl2_to_bl31_params->bl33_ep_info :
					bl2_to_bl31_params->bl32_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL31 specific platform actions. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables. On the RCAR
 * we know that BL2 has populated the parameters. So we just use
 * the reference passed in 'from_bl2' instead of copying. The 'data' parameter
 * is not used since all the information is contained in 'from_bl2'. Also, BL2
 * has flushed this information to memory, so we are guaranteed to pick up good
 * data
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
		void *plat_params_from_bl2)
{
	uint32_t cluster_type;

	/* Initialize the log area to provide early debug support */
	console_init(1U, 0U, 0U);

	NOTICE("BL3-1 : Rev.%s\n",version_of_renesas);

	/* Check params passed from BL2 should not be NULL,
	 */
	if ((NULL == from_bl2) || ((uint8_t)PARAM_BL31 != from_bl2->h.type) ||
	    ((uint8_t)VERSION_1 > from_bl2->h.version)) {
		panic();
	}

	bl2_to_bl31_params = from_bl2;

	cluster_type = rcar_bl31_get_cluster();
	if (RCAR_CLUSTER_A53A57 == cluster_type) {
		/*
		 * Initialize CCI for this cluster during cold boot.
		 * No need for locks as no other CPU is active.
		 */
		rcar_cci_init();
		/*
		 * Enable CCI coherency for the primary CPU's cluster
		 * RCAR PSCI code will enable coherency for other clusters.
		 */
		rcar_cci_enable();
	}

}

/*******************************************************************************
 * Initialize the gic, configure the CLCD and zero out variables needed by the
 * secondaries to boot up correctly.
 ******************************************************************************/
void bl31_platform_setup(void)
{

	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();

	/* Enable and initialize the System level generic timer */
	mmio_write_32(RCAR_CNTC_BASE + CNTCR_OFF, CNTCR_FCREQ(0) | CNTCR_EN);

	/* Intialize the power controller */
	rcar_pwrc_setup();

	/* Topologies are best known to the platform. */
	rcar_setup_topology();

	/* Get the mpidr for boot cpu */
	rcar_boot_mpidr = read_mpidr_el1() & 0x0000ffffU;
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	rcar_configure_mmu_el3(BL31_RO_BASE, (BL31_END - BL31_RO_BASE),
			BL31_RO_BASE,
			BL31_RO_LIMIT
#if USE_COHERENT_MEM
			, BL31_COHERENT_RAM_BASE,
			BL31_COHERENT_RAM_LIMIT
#endif
	);

}

/*******************************************************************************
 * There check whether duplication of physical address is valid or not.
 ******************************************************************************/
uint32_t bl31_plat_mmu_pa_chk(uint32_t pa_flg, uintptr_t chk_va, uint64_t chk_pa)
{
	if ((DEVICE_SRAM_SHADOW_BASE == chk_va) &&
	    (DEVICE_SRAM_BASE_U == chk_pa)) {
		pa_flg = 1U;
	}
	return pa_flg;
}

/*******************************************************************************
 * Instead of svc_migrate_info in RCAR.
 * returned PSCI_TOS_NOT_UP_MIG_CAP and boot MPIDR.
 ******************************************************************************/
uint32_t bl31_plat_cpu_migrate_info(u_register_t *resident_cpu)
{
	*resident_cpu = rcar_boot_mpidr;
	return (uint32_t)PSCI_TOS_NOT_UP_MIG_CAP;
}

/*******************************************************************************
 * There check whether CPU_OFF is OK or not.
 ******************************************************************************/
int32_t bl31_plat_denied_cpu_off_chk(void)
{
	int32_t rc = PSCI_E_SUCCESS;
	uint64_t tmp_mpidr;

	tmp_mpidr = read_mpidr_el1() & 0x0000ffffU;

	if (tmp_mpidr == rcar_boot_mpidr) {
		rc = PSCI_E_DENIED;
	}
	return rc;
}

/*******************************************************************************
 * Check boot_mpidr(CPU0).
 * If the CPU is the same as CPU0, return RCAR_MPIDRCHK_BOOTCPU,
 * otherwise it returns RCAR_MPIDRCHK_NOT_BOOTCPU.
 ******************************************************************************/
uint32_t bl31_plat_boot_mpidr_chk(void)
{
	uint32_t rc = RCAR_MPIDRCHK_NOT_BOOTCPU;
	uint64_t tmp_mpidr;

	tmp_mpidr = read_mpidr_el1() & 0x0000ffffU;

	if (tmp_mpidr == rcar_boot_mpidr) {
		rc = RCAR_MPIDRCHK_BOOTCPU;
	}
	return rc;
}

