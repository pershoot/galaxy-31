/*
 * drivers/video/tegra/dc/clock.c
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * Copyright (c) 2010-2012, NVIDIA CORPORATION, All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/err.h>
#include <linux/types.h>
#include <linux/clk.h>

#include <mach/clk.h>
#include <mach/dc.h>

#include "dc_reg.h"
#include "dc_priv.h"

unsigned long tegra_dc_pclk_round_rate(struct tegra_dc *dc, int pclk)
{
	unsigned long rate;
	unsigned long div;

	rate = tegra_dc_clk_get_rate(dc);

	div = DIV_ROUND_CLOSEST(rate * 2, pclk);

	if (div < 2)
		return 0;

	return rate * 2 / div;
}

static unsigned long tegra_dc_pclk_predict_rate(struct clk *parent, int pclk)
{
	unsigned long rate;
	unsigned long div;

	rate = clk_get_rate(parent);

	div = DIV_ROUND_CLOSEST(rate * 2, pclk);

	if (div < 2)
		return 0;

	return rate * 2 / div;
}

void tegra_dc_setup_clk(struct tegra_dc *dc, struct clk *clk)
{
	int pclk;

	if (dc->out->type == TEGRA_DC_OUT_RGB) {
		unsigned long rate;

                struct clk *pll_c_clk = clk_get_sys(NULL, "pll_c");

                struct clk *pll_p_clk = clk_get_sys(NULL, "pll_p");

                switch (dc->mode.pclk) {
                case 68941176:
                        rate = 586000000;
                        if (clk_get_parent(clk) != pll_c_clk)
                                clk_set_parent(clk, pll_c_clk);

                        if (rate != clk_get_rate(pll_c_clk))
                                clk_set_rate(pll_c_clk, rate);
                break;

                case 70000000:
                case 74666667:
                        rate = 560000000;
                        if (clk_get_parent(clk) != pll_c_clk)
                                clk_set_parent(clk, pll_c_clk);

                        if (rate != clk_get_rate(pll_c_clk))
                                clk_set_rate(pll_c_clk, rate);
                break;
                case 72000000:
                        if (clk_get_parent(clk) != pll_p_clk)
                                clk_set_parent(clk, pll_p_clk);

                break;

                case 76000000:
                        rate = 570000000;
                        if (clk_get_parent(clk) != pll_c_clk)
                                clk_set_parent(clk, pll_c_clk);

                        if (rate != clk_get_rate(pll_c_clk))
                                clk_set_rate(pll_c_clk, rate);
                break;

                case 75500000:
                        rate = 453000000;
                        if (clk_get_parent(clk) != pll_c_clk)
                                clk_set_parent(clk, pll_c_clk);

                        if (rate != clk_get_rate(pll_c_clk))
                                clk_set_rate(pll_c_clk, rate);
                break;
                }
	}

	if (dc->out->type == TEGRA_DC_OUT_HDMI) {
		unsigned long rate;
		struct clk *parent_clk = clk_get_sys(NULL,
			dc->out->parent_clk ? : "pll_d_out0");
		struct clk *base_clk = clk_get_parent(parent_clk);

		/*
		 * Providing dynamic frequency rate setting for T20/T30 HDMI.
		 * The required rate needs to be setup at 4x multiplier,
		 * as out0 is 1/2 of the actual PLL output.
		 */

		rate = dc->mode.pclk * 4;
		if (rate != clk_get_rate(base_clk))
			clk_set_rate(base_clk, rate);

		if (clk_get_parent(clk) != parent_clk)
			clk_set_parent(clk, parent_clk);
	}

	if (dc->out->type == TEGRA_DC_OUT_DSI) {
		unsigned long rate;
		struct clk *parent_clk;
		struct clk *base_clk;

		if (clk == dc->clk) {
			parent_clk = clk_get_sys(NULL,
					dc->out->parent_clk ? : "pll_d_out0");
			base_clk = clk_get_parent(parent_clk);
			tegra_clk_cfg_ex(base_clk,
					TEGRA_CLK_PLLD_DSI_OUT_ENB, 1);
		} else {
			if (dc->pdata->default_out->dsi->dsi_instance) {
				parent_clk = clk_get_sys(NULL,
					dc->out->parent_clk ? : "pll_d2_out0");
				base_clk = clk_get_parent(parent_clk);
				tegra_clk_cfg_ex(base_clk,
						TEGRA_CLK_PLLD_CSI_OUT_ENB, 1);
			} else {
				parent_clk = clk_get_sys(NULL,
					dc->out->parent_clk ? : "pll_d_out0");
				base_clk = clk_get_parent(parent_clk);
				tegra_clk_cfg_ex(base_clk,
						TEGRA_CLK_PLLD_DSI_OUT_ENB, 1);
			}
		}

		rate = dc->mode.pclk * dc->shift_clk_div * 2;
		if (rate != clk_get_rate(base_clk))
			clk_set_rate(base_clk, rate);

		if (clk_get_parent(clk) != parent_clk)
			clk_set_parent(clk, parent_clk);
	}

	pclk = tegra_dc_pclk_round_rate(dc, dc->mode.pclk);
	tegra_dvfs_set_rate(clk, pclk);
}
