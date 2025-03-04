import pandas
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

df = pandas.read_csv("result.csv", header=None)
print(df)
df = df[::-1]

fig = plt.figure()
ax1 = fig.add_subplot(1, 1, 1)
ax1.tick_params(axis='x', labelrotation=90)

df.plot.barh(
    x=0,
    y=1,
    label="obj per second",
    xlabel="obj per second",
    ylabel="",
    ax=ax1,
)

plt.title("json generator benchmark")
plt.tight_layout()
plt.savefig("result.png")
