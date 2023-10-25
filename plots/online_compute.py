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

## Our protocol (dishonest)
dishonest_server = [
    615,
1310,
2193
]

dishonest_total = [
   772.083,
1539.3,
2513
]

dishonest_client = [(x-y) for (x,y) in zip(dishonest_total, dishonest_server)]

dishonest_client = [x / 1000.0 for x in dishonest_client]
dishonest_server = [x / 1000.0 for x in dishonest_server]

honest_server = [
  430,
859,
1714
]

honest_total = [
  560.877,
1043,
1974
]

honest_client = [(x-y) for (x,y) in zip(honest_total, honest_server)]

honest_client = [x / 1000.0 for x in honest_client]
honest_server = [x / 1000.0 for x in honest_server]


## SimplePir protocol data

# 1-bit entries
# 1GB : 117.236
original_server = [
  446,
889,
1937
]


original_total = [
  586,
1086,
2231
]

original_client = [(x-y) for (x,y) in zip(original_total, original_server)]

original_client = [x / 1000.0 for x in original_client]  ## original data in ms
original_server = [x / 1000.0 for x in original_server]

simple_pir_64_server = [
    421,
    857.5,
    1719.98
]

simple_pir_64_total = [
    544.55,
1033.87,
1970.786
]

simple_pir_64_client = [(x-y) for (x,y) in zip(simple_pir_64_total, simple_pir_64_server)]

simple_pir_64_client = [x / 1000.0 for x in simple_pir_64_client]  ## simple_pir_64 data in ms
simple_pir_64_server = [x / 1000.0 for x in simple_pir_64_server]


fig = plt.figure(figsize=(20, 10))
x_labels = [
    "4",
    "8",
    "16",
]
x = np.arange(len(x_labels))


bar_width = 0.15
linewidth = 3

original_bar_list = [
    plt.bar(
        x - (3/2)*bar_width,
        original_server,
        align="center",
        width=bar_width,
        label=simplepir_label,
        color=simplepir_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
    plt.bar(
        x - (3/2)*bar_width,
        # original_upload,
        original_client,
        bottom=original_server,
        align="center",
        width=bar_width,
        color=simplepir_color,
        edgecolor="black",
        linewidth=linewidth,
        hatch="//",
    ),
]

simplepir_64_bar_list = [
    plt.bar(
        x - (1/2)*bar_width,
        simple_pir_64_server,
        align="center",
        width=bar_width,
        label=simplepir_64_label,
        color=simplepir_64_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
    plt.bar(
        x - (1/2)*bar_width,
        # simplepir_64_upload,
        simple_pir_64_client,
        bottom=simple_pir_64_server,
        align="center",
        width=bar_width,
        color=simplepir_64_color,
        edgecolor="black",
        linewidth=linewidth,
        hatch="//",
    ),
]

our_honest_bar_list = [
    plt.bar(
        x + bar_width/2,
        # upload,
        honest_server,
        align="center",
        width=bar_width,
        label=our_honest_label,
        color=our_honest_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
    plt.bar(
        x + bar_width/2,
        # upload,
        honest_client,
        bottom=honest_server,
        align="center",
        width=bar_width,
        color=our_honest_color,
        edgecolor="black",
        linewidth=linewidth,
        hatch="//",
    )
]


dishonest_bar_list = [
    plt.bar(
        x + (3/2)*bar_width,
        # upload,
        dishonest_server,
        align="center",
        width=bar_width,
        label=our_dishonest_label,
        color=our_dishonest_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
    plt.bar(
        x + (3/2)*bar_width,
        dishonest_client,
        bottom=dishonest_server,
        align="center",
        width=bar_width,
        color=our_dishonest_color,
        edgecolor="black",
        linewidth=linewidth,
        hatch="//",
    ),
]


plt.title("Online Computation Time (in seconds)", **axis_font, weight="bold")
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
plt.savefig("online_computation.svg", format="svg", bbox_inches='tight')

plt.show()
