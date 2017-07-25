/*
 * ASoC Driver for Realtek Codec
 *
 * Author:	Oder Chiou <oder_chiou@realtek.com>
 *		Copyright 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/jack.h>

static int snd_realtek_rpi_hw_params(struct snd_pcm_substream *substream,
				       struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	return snd_soc_dai_set_bclk_ratio(cpu_dai, 32*2);
}

/* machine stream operations */
static struct snd_soc_ops snd_realtek_rpi_ops = {
	.hw_params = snd_realtek_rpi_hw_params,
};

static struct snd_soc_dai_link snd_realtek_rpi_dai[] = {
{
	.name		= "Realtek-codec",
	.stream_name	= "Realtek-codec-DAC HiFi",
	.cpu_dai_name	= "bcm2835-i2s.0",
	.codec_dai_name	= "snd-soc-dummy-dai",
	.platform_name	= "bcm2835-i2s.0",
	.codec_name	= "snd-soc-dummy",
	.dai_fmt	= SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
				SND_SOC_DAIFMT_CBS_CFS,
	.ops		= &snd_realtek_rpi_ops,
},
};

/* audio machine driver */
static struct snd_soc_card snd_realtek = {
	.name         = "snd_realtek",
	.owner        = THIS_MODULE,
	.dai_link     = snd_realtek_rpi_dai,
	.num_links    = ARRAY_SIZE(snd_realtek_rpi_dai),
};

static int snd_realtek_rpi_probe(struct platform_device *pdev)
{
	int ret = 0;

	snd_realtek.dev = &pdev->dev;

	if (pdev->dev.of_node) {
		struct device_node *i2s_node;
		struct snd_soc_dai_link *dai = &snd_realtek_rpi_dai[0];

		i2s_node = of_parse_phandle(pdev->dev.of_node,
			"i2s-controller", 0);

		if (i2s_node) {
			dai->cpu_dai_name = NULL;
			dai->cpu_of_node = i2s_node;
			dai->platform_name = NULL;
			dai->platform_of_node = i2s_node;
		}
	}

	ret = snd_soc_register_card(&snd_realtek);
	if (ret && ret != -EPROBE_DEFER)
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n",
			ret);

	return ret;
}

static int snd_realtek_rpi_remove(struct platform_device *pdev)
{
	return snd_soc_unregister_card(&snd_realtek);
}

static const struct of_device_id snd_realtek_rpi_of_match[] = {
	{ .compatible = "realtek-codec", },
	{},
};
MODULE_DEVICE_TABLE(of, snd_realtek_rpi_of_match);

static struct platform_driver snd_realtek_rpi_driver = {
	.driver = {
		.name	= "snd-realtek-codec",
		.owner	= THIS_MODULE,
		.of_match_table = snd_realtek_rpi_of_match,
	},
	.probe		= snd_realtek_rpi_probe,
	.remove		= snd_realtek_rpi_remove,
};

module_platform_driver(snd_realtek_rpi_driver);

MODULE_AUTHOR("Oder Chiou <oder_chiou@realtek.com>");
MODULE_DESCRIPTION("ASoC Driver for Realtek Codec");
MODULE_LICENSE("GPL v2");
