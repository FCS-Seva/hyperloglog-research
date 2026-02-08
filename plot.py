import csv
import matplotlib.pyplot as plt

x, truev, estv = [], [], []
with open("stream0.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        x.append(int(row["percent"]))
        truev.append(float(row["trueF0"]))
        estv.append(float(row["estimateNt"]))

plt.figure()
plt.plot(x, truev, label="F0^t (true)")
plt.plot(x, estv, label="Nt (HLL)")
plt.xlabel("percent of stream processed")
plt.ylabel("unique count")
plt.legend()
plt.tight_layout()
plt.savefig("graph1.png")

x2, meanNt, sdNt = [], [], []
with open("stats.csv", newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        x2.append(int(row["percent"]))
        meanNt.append(float(row["meanNt"]))
        sdNt.append(float(row["sdNt"]))

low = [m - s for m, s in zip(meanNt, sdNt)]
high = [m + s for m, s in zip(meanNt, sdNt)]

plt.figure()
plt.plot(x2, meanNt, label="E(Nt)")
plt.fill_between(x2, low, high, alpha=0.25, label="E(Nt) ± σ")
plt.xlabel("percent of stream processed")
plt.ylabel("estimated unique count")
plt.legend()
plt.tight_layout()
plt.savefig("graph2.png")

print("OK: graph1.png, graph2.png")
