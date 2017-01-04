#!/usr/bin/python

import subprocess
import os
import sys
import time


def execPageRank(resultFile, graphFile, threadNum):
    """Function that calls the serial and the pthreads version
       of the PageRank Algorithm and writes their output in
       a txt file that will be parsed to extract information
       for their performance

    :resultFile: The file where the results will be writen.
    :graphFile: The name of the file that contains the current graph
                we wish to use.
    :threadNum: The number of threads for the pthreads version.
    :returns: None
    """
    result = subprocess.call("./pagerank_serial.exe " + graphFile,
                             shell=True, stdout=resultFile)
    result = subprocess.call("./pagerank_pthreads.exe " +
                             graphFile + " " + str(threadNum),
                             shell=True, stdout=resultFile)


def calcStats(resultFile):
    parallelTime = []
    serialTime = []
    serialIters = []
    parallelIters = []

    avgSerialTime = 0
    avgParallelTime = 0
    avgSpeedUp = 0
    avgIters = 0

    for line in resultFile:
        if "Serial" in line:
            posStart = line.find("<")
            posEnd = line.find(">")
            if "Time" in line:
                serialTime.append(float(line[posStart + 1:posEnd]))
            elif "Iterations" in line:
                serialIters.append(float(line[posStart + 1:posEnd]))
        elif "Parallel" in line:
            posStart = line.find("<")
            posEnd = line.find(">")
            if "Time" in line:
                parallelTime.append(float(line[posStart + 1:posEnd - 1]))
                pass
            elif "Iterations" in line:
                parallelIters.append(float(line[posStart + 1:posEnd]))
        else:
            continue
    for pTime, sTime in zip(parallelTime, serialTime):
        avgSpeedUp += sTime / pTime
        avgSerialTime += sTime
        avgParallelTime += pTime

    avgSpeedUp = avgSpeedUp / len(parallelTime)
    avgSerialTime = avgSerialTime / len(parallelTime)
    avgParallelTime = avgParallelTime / len(parallelTime)

    return avgSpeedUp, avgSerialTime, avgParallelTime


def main():
    print "Starting to collect data!"

    webGraphs = os.listdir("tmp")
    webGraphs = [x for x in webGraphs if ".txt" in x]
    threads = [2 ** i for i in range(1, 6)]

    if "Makefile" not in os.listdir(os.getcwd()):
        print "No Makefile in current folder. Exiting!"
        sys.exit(-1)
    result = subprocess.call(["make", "clean"])

    if result != 0:
        print "Make clean was not executed successfully!"
        sys.exit(-1)

    result = subprocess.call("make")
    if result != 0:
        print "Make command was not executed successfully!"
        sys.exit(-1)

    if not os.path.exists("results"):
        os.mkdir("results")

    results = dict()
    for graphFile in webGraphs:
        pos = graphFile.find(".")
        currentGraph = graphFile[:pos]
        data = dict()
        data["SpeedUp"] = []
        data["AvgParallelTime"] = []
        data["AvgSerialTime"] = []

        for threadNum in threads:
            # Open a temporary file to store the results.
            with open("results/tmp.txt", "w") as tempFile:
                for i in range(0, 10):
                    execPageRank(tempFile, graphFile, threadNum)

            with open("results/tmp.txt", "r") as resultFile:
                avgSpeedUp, avgSTime, avgPTime = calcStats(resultFile)
            data["SpeedUp"].append(avgSpeedUp)
            data["AvgParallelTime"].append(avgPTime)
            data["AvgSerialTime"].append(avgSTime)
        results[currentGraph] = data

    with open("results/speedUpResults.txt", "w") as speedUpFile:
        for key, item in results.iteritems():
            speedUpFile.write(key + ": ")
            for speedUp in item["SpeedUp"]:
                print speedUp
                speedUpFile.write(str(speedUp) + ", ")
            speedUpFile.write("\n")

    with open("results/avgParallelTime.txt", "w") as avgParallelFile:
        for key, item in results.iteritems():
            avgParallelFile.write(key + ": ")
            for avgTime in item["AvgParallelTime"]:
                avgParallelFile.write(str(avgTime) + ", ")
            avgParallelFile.write("\n")

    with open("results/avgSerialTime.txt", "w") as avgSerialFile:
        for key, item in results.iteritems():
            avgSerialFile.write(key + ": ")
            for avgTime in item["AvgSerialTime"]:
                avgSerialFile.write(str(avgTime) + ", ")
            avgSerialFile.write("\n")


if __name__ == "__main__":
    main()
