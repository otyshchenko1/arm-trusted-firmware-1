/*
 * Copyright (c) 2016,2017 ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2015-2017 Renesas Electronics Corporation. All rights reserved.
 * Copyright (c) 2017 EPAM Systems Inc. All rights reserved.
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

#include "rcar_def.h"
#include "rcar_private.h"

struct scpi_clock {
	uint32_t min_freq;
	uint32_t max_freq;
	uint32_t (*getter)(uint32_t);
	uint32_t (*setter)(uint32_t, uint32_t);
	uint32_t reg_addr;
	const char *name;
	uint16_t clockid;
};

struct scpi_clock rcar_clocks[] = {
	0
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static struct scpi_clock *get_rcar_clock(int clocknr)
{
	if (clocknr < 0 || clocknr >= ARRAY_SIZE(rcar_clocks))
		return NULL;

	return &rcar_clocks[clocknr];
}

uint32_t rcar_clock_get_min_rate(int clocknr)
{
	struct scpi_clock *clk = get_rcar_clock(clocknr);

	if (!clk)
		return ~0;

	return clk->min_freq;
}

uint32_t rcar_clock_get_max_rate(int clocknr)
{
	struct scpi_clock *clk = get_rcar_clock(clocknr);

	if (!clk)
		return ~0;

	return clk->max_freq;
}

const char* rcar_clock_get_name(int clocknr)
{
	struct scpi_clock *clk = get_rcar_clock(clocknr);

	if (!clk)
		return NULL;

	return clk->name;
}

uint32_t rcar_clock_get_rate(int clocknr)
{
	struct scpi_clock *clk = get_rcar_clock(clocknr);

	if (!clk)
		return ~0;

	return clk->getter(clk->reg_addr);
}

int rcar_clock_set_rate(int clocknr, uint32_t freq)
{
	struct scpi_clock *clk = get_rcar_clock(clocknr);

	if (!clk)
		return ~0;

	return clk->setter(clk->reg_addr, freq);
}

int rcar_clock_nr_clocks(void)
{
	return ARRAY_SIZE(rcar_clocks);
}
