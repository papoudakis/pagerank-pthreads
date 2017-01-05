# PageRank with PThreads

This is a parallel implementation on PageRank algorithm using C++ and PThreads.

## Execution 
In order to compile the source code just run:
```
make
```
For running the serial version run:
```
./pagerank_serial.exe graphFileName
```
and for parallel version:
```
./pagerank_pthreads.exe graphFileName
```
where graphFileName is the name of the graph that PageRank will use. Many free 
graphs can be downloaded from [here](http://snap.stanford.edu/data/)

## Results
The plot below shows the execution time of serial and parallel implemantation for 4 different graphs


![Screenshot](https://cloud.githubusercontent.com/assets/9269275/21663587/d3e95778-d2e9-11e6-9ab4-c3beec9ce706.png)

## Extras
Python and Matlab files are used for testing, creating new graphs and making plots.

## Authors
*  [Vassilis Choutas](https://github.com/vasilish) 
*  [Giorgos Papoudakis](https://github.com/papoudakis)
