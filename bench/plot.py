import pandas
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

df = pandas.read_csv("result.csv", header=None)
print(df)
df = df[::-1]

fig = plt.figure()
ax1 = fig.add_subplot(1, 1, 1)
ax2 = ax1.twiny()

df.plot.barh(
    x=0,
    y=1,
    label="obj per second",
    xlabel="obj per second",
    ylabel="",
    ax=ax1,
)
ax1.tick_params(axis='x', labelrotation=90)

df.plot.barh(
    x=0,
    y=2,
    label="peak heap usage",
    color="#a98d1980",
    position=1,
    xlabel="byte",
    ylabel="",
    ax=ax2,
)

lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend().remove()
ax2.legend(lines1 + lines2, labels1 + labels2, loc="upper right")

plt.title("json generator benchmark")
plt.tight_layout()
plt.savefig("result.png")
