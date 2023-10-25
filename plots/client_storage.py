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
dishonest_totals = [
    0.69300878,
1.010070801,
1.428673159,
2.086429105,
2.951094301,
4.174118951,
6.10944267
]

honest_totals = [
   0.5727978489,
0.8101784024,
1.145940967,
1.620845088,
2.339149216,
3.308557858,
4.679707979
]

## SimplePir protocol data

# 1-bit entries
# 1GB : 117.236
original_totals = [
    0.2357063293,
0.3333358765,
0.5,
0.7071113586,
1,
1.414218903,
2
]


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
        x - bar_width,
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

our_honest_bar_list = [
    plt.bar(
        x,
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
        x + bar_width,
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


plt.title("Client Local Storage (in GiB)", **axis_font, weight="bold")
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
plt.savefig("client_local_storage.svg", format="svg", bbox_inches='tight')

plt.show()
