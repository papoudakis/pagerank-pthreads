import matplotlib.pyplot as plt

threads = [2**i for i in range(1, 6)]

pTime = dict()
sTime = dict()
speedUp = dict()
with open("results/avgParallelTime.txt") as pTimeFile:
    for line in pTimeFile:
        tokens = line.strip("\n ").split(":")
        times = tokens[1].strip(" ").split(",")
        times = [x for x in times if len(x) > 0]
        times = [float(x) for x in times]
        pTime[tokens[0]] = times
with open("results/avgSerialTime.txt") as sTimeFile:
    for line in sTimeFile:
        tokens = line.strip("\n ").split(":")
        times = tokens[1].strip(" ").split(",")
        times = [x for x in times if len(x) > 0]
        times = [float(x) for x in times]
        sTime[tokens[0]] = times
with open("results/speedUpResults.txt") as sFile:
    for line in sFile:
        tokens = line.strip("\n ").split(":")
        times = tokens[1].strip(" ").split(",")
        times = [x for x in times if len(x) > 0]
        times = [float(x) for x in times]
        speedUp[tokens[0]] = times


plt.figure(0)
for key, val in speedUp.iteritems():
    plt.plot(threads, val, '-')
    plt.xlabel("Number of Threads")
    plt.ylabel("Code SpeedUp (Serial / Parallel Time)")
    plt.title("Parallel Code SpeedUp Graph", fontweight="bold", fontsize=16)

graphNames = [key for key in speedUp.keys()]
leg = plt.legend(graphNames, loc="lower right", fancybox=True)
# Make legend Transparent
leg.get_frame().set_alpha(0.5)

plt.savefig("speedup_plot.png")

plt.figure(1)

graphNames = []

for key, val in pTime.iteritems():
    plt.plot(threads, val, '-', threads, sTime[key])
    graphNames.append(key)
    graphNames.append(key + " Serial")
    plt.xlabel("Number of Threads")
    plt.ylabel("Time (seconds)")
    plt.title("Execution Time Graph", fontweight="bold", fontsize=16)

leg = plt.legend(graphNames, loc="top right", fancybox=True)
# Make legend Transparent
leg.get_frame().set_alpha(0.5)

plt.savefig("time_plot.png")


plt.show()
