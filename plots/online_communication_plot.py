import numpy as np
import matplotlib.pyplot as plt
import matplotlib.font_manager as font_manager
from textwrap import wrap

from plot_colors import *


fontname = "serif"
axis_font = {"fontname": fontname, "size": "38"}
# legend_font = {"fontname": fontname, "size": "28"}
font = font_manager.FontProperties(
    # family="Times", weight="bold", style="normal", size=30
    # family="Times",
    family=fontname,
    style="normal",
    size=32,
)

## Our protocol data

# 1-bit entries, dishonest hint
# upload = [77.8515625,
# 108.5390625,160.8125,224.203125,332.9140625,464.1484375,656.40625]
# downloads = [3324.217987,4737.847334,6871.159389,9792.927292,14234.96088,20287.6043,
# 28896.18142]
# totals = [x+y for x,y in zip(upload, downloads)]
# upload = [56.3625, 53.632, 36.0341, 36.0341]
# totals = [129.219, 121.458, 79.2401, 79.2401]
# downloads = [(x - y) for (x, y) in zip(totals, upload)]

# upload = np.array(upload)
# downloads = np.array(downloads)
# totals = np.array(totals)

## Our protocol (dishonest)
dishonest_totals = [702.46875,
1024,
1448.15625,
2115.171875,
2991.296875,
4230.34375,
6192.578125]

dishonest_totals = [x / 1000.0 for x in dishonest_totals]

honest_totals = [
    579.265625,
819.203125,
1158.53125,
1638.40625,
2364.828125,
3344.375,
4729.65625,
]

honest_totals = [x / 1000.0 for x in honest_totals]


# 1-bit entries, honest hint
# hh_upload = [66.648, 94.256, 114.008]
# hh_downloads = [3218, 4580, 5634]
# hh_totals = [x+y for x,y in zip(hh_upload, hh_downloads)]

# hh_totals = np.array(hh_totals)

## SimplePir protocol data

# 1-bit entries
# 1GB : 117.236
original_totals = [482.71875,
682.6679688,
1024,
1448.15625,
2048,
2896.3125,
4096]

original_totals = [x / pow(2, 10) for x in original_totals]

simple_pir_64_totals = [
    579.265625,
819.203125,
1158.53125,
1638.40625,
2364.78125,
3344.375,
4729.65625
]

simple_pir_64_totals = [x / pow(2, 10) for x in simple_pir_64_totals]  ## data in KiB


## APIR protocol data

# # 1-bit entries
# apir_upload = [0,0,0,0,11585.2375, 16384, 23170.47501]
# apir_download = apir_upload
# apir_totals = [x+y for x,y in zip(apir_upload, apir_download)]

# # 1-bit entries
# apir_plus_upload = [24618.62969,
# 38912,
# 60822.49689,
# 94208,
# 144815.4688,
# 221184,
# 359142.3626]
# apir_plus_download = apir_plus_upload
# apir_plus_totals = [x+y for x,y in zip(apir_plus_upload, apir_plus_download)]


fig = plt.figure(figsize=(20, 10))
x_labels = [
    "4",
    "8",
    "16",
    "32",
    "64",
    "128",
    "256"
]
x = np.arange(len(x_labels))


bar_width = 0.15
linewidth = 3

original_bar_list = [
    plt.bar(
        x - (3/2)*bar_width,
        # original_upload,
        original_totals,
        align="center",
        width=bar_width,
        # label="Upload",
        label=simplepir_label,
        color=simplepir_color,
        # color="darkred",
        # color="tomato",
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
]

simple_pir_64_bars = [
    plt.bar(
        x - (bar_width/2),
        # original_upload,
        simple_pir_64_totals,
        align="center",
        width=bar_width,
        # label="Upload",
        label=simplepir_64_label,
        color=simplepir_64_color,
        # color="darkred",
        # color="tomato",
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
]

our_honest_bar_list = [
    plt.bar(
        x+(bar_width/2),
        # upload,
        honest_totals,
        align="center",
        width=bar_width,
        # label="Uploads",
        label=our_honest_label,
        # color="yellow",
        color=our_honest_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
]


dishonest_bar_list = [
    plt.bar(
        x + (3/2)*bar_width,
        # upload,
        dishonest_totals,
        align="center",
        width=bar_width,
        # label="Uploads",
        label=our_dishonest_label,
        # color="palegoldenrod",
        # color="green",
        # color="forestgreen",
        color=our_dishonest_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
]


plt.title("Online Communication (in MiB)", **axis_font, weight="bold")
plt.xlabel("Database Size (GiB)", **axis_font)
# plt.ylabel("Communication (in KiB)", **axis_font)
plt.legend(prop=font)
# plt.legend(prop=font, bbox_to_anchor=(1.04,1), loc="upper left")
plt.xticks(ticks=x, labels=x_labels, **axis_font)
# fig.set_xticklabels(x_labels)
plt.yticks(fontname=fontname, fontsize=32)
# plt.yscale("log")
ax = plt.subplot() 
# ax.set_yscale("log") 
# ax.get_yaxis().set_major_formatter(mpl.ticker.LogLocator(base=10.0, subs=[1.0], numdecs=4, numticks=15))
# ax
# yticks = [0, 10, pow(10,2), pow(10,3), pow(10,4), pow(10,5)]
# ax.set_yticks(yticks)
# ax.get_yaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
# yticks = [i*2000 for i in range(7)]
# ax.set_yticks(yticks)
# ax.locator_params(axis='y',tight=True, numticks=10)
# plt.savefig("/Users/leo/simplepir/plots/CommPlot1bit.png", bbox_inches="tight")
plt.plot()
plt.savefig("online_comm.svg", format="svg", bbox_inches='tight')

plt.show()
