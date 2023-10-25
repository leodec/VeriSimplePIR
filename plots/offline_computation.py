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
    55410,
115620,
205620
]

dishonest_client = [
9306.52,
13582,
18960
]

dishonest_client = [x / 1000.0 for x in dishonest_client]
dishonest_server = [x / 1000.0 for x in dishonest_server]

honest_server = [
  35857.5,
71655,
143070
]

honest_client = [
7753.88,
10900,
15400
]

honest_client = [x / 1000.0 for x in honest_client]
honest_server = [x / 1000.0 for x in honest_server]


fig = plt.figure(figsize=(20, 10))
x_labels = [
    "4",
    "8",
    "16",
]
x = np.arange(len(x_labels))


bar_width = 0.15
linewidth = 3


our_honest_bar_list = [
    plt.bar(
        x - bar_width/2,
        # upload,
        honest_server,
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
    plt.bar(
        x - bar_width/2,
        # upload,
        honest_client,
        bottom=honest_server,
        align="center",
        width=bar_width,
        # label="Uploads",
        # label=our_honest_label + ", client work",
        # color="yellow",
        color=our_honest_color,
        edgecolor="black",
        linewidth=linewidth,
        hatch="//",
    )
]


dishonest_bar_list = [
    plt.bar(
        x + bar_width/2,
        # upload,
        dishonest_server,
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
    plt.bar(
        x + bar_width/2,
        # upload,
        dishonest_client,
        bottom=dishonest_server,
        align="center",
        width=bar_width,
        # label=our_dishonest_label + ", client work",
        # color="palegoldenrod",
        # color="green",
        # color="forestgreen",
        color=our_dishonest_color,
        edgecolor="black",
        linewidth=linewidth,
        hatch="//",
    ),
]


plt.title("Offline Computation Time (in seconds)", **axis_font, weight="bold")
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
plt.savefig("offline_computation.svg", format="svg", bbox_inches='tight')

plt.show()
