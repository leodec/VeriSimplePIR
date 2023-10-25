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
dishonest_ephemeral = [
    0.9178990179,
1.346684717,
1.916543304,
2.817260938,
4.009070584,
5.704872504,
8.403988544
]

dishonest_permanent = [
    0.69300878,
1.010070801,
1.428673159,
2.086429105,
2.951094301,
4.174118951,
6.10944267
]

dishonest_totals = [(x+y) for (x,y) in zip(dishonest_ephemeral, dishonest_permanent)]

honest_permanent = [
   0.5727978489,
0.8101784024,
1.145940967,
1.620845088,
2.339149216,
3.308557858,
4.679707979
]


honest_ephemeral = [
   0.756994572,
1.077360767,
1.533255522,
2.181968235,
3.168801628,
4.509174804,
6.416257452
]

honest_totals = [(x+y) for (x,y) in zip(honest_ephemeral, honest_permanent)]


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

simple_pir_64_totals = [
    0.5657958984,
0.8000488281,
1.131469727,
1.600097656,
2.309356689,
3.266113281,
4.618896484
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
        x - (3/2)*bar_width,
        original_totals,
        align="center",
        width=bar_width,
        label=simplepir_label,
        color=simplepir_color,
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
        label=simplepir_64_label,
        color=simplepir_64_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
    ),
]

our_honest_bar_list = [
    plt.bar(
        x + bar_width/2,
        # upload,
        honest_permanent,
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
        honest_ephemeral,
        bottom=honest_permanent,
        align="center",
        width=bar_width,
        color=our_honest_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
        hatch="XX",
    )
]


dishonest_bar_list = [
    plt.bar(
        x + (3/2)*bar_width,
        # upload,
        dishonest_permanent,
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
        dishonest_ephemeral,
        bottom=dishonest_permanent,
        align="center",
        width=bar_width,
        color=our_dishonest_color,
        edgecolor="black",
        linewidth=linewidth,
        # hatch="//",
        hatch="XX",
    ),
]


plt.title("Offline Communication (in GiB)", **axis_font, weight="bold")
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
plt.savefig("offline_total_communication.svg", format="svg", bbox_inches='tight')

plt.show()
