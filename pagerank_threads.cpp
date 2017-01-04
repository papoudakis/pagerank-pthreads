#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <limits>
#include <assert.h>
#include <pthread.h>
#include "sys/time.h"
#include <unistd.h>

#include "utils.h"


int NUM_THREADS;
pthread_spinlock_t sumMutex;
pthread_barrier_t barrier;
pthread_spinlock_t terminationLock;

struct threadData
{
  std::vector<int>* L;
  int* c;
  int* d;
  int N;
  float* x;
  float* z;
  float* E;
  int chunk;
  int threadId;
  int start;
};

float sum;
int pageRankIterNum = 0;
int threadsFinished = 0;

void* threadedPageRank(void* arg)
{
  struct threadData* dataPtr = static_cast<threadData*>(arg);

  // The number of elements that will be processed by this thread.
  int chunk = dataPtr->chunk;
  int start = dataPtr->start;
  // The number of nodes in the graph.
  int N = dataPtr->N;
  float* x = dataPtr->x;
  float* z = dataPtr->z;
  float* E = dataPtr->E;

  // Get the adjacency matrix
  std::vector<int>* L = dataPtr->L;

  // The array containing the number of outbound connections for each
  // vertex of the graph.
  int* c = dataPtr->c;
  // The array containing the number of incoming connections for each
  // vertex of the graph.
  int* d = dataPtr->d;

  int threadId = dataPtr->threadId;

  float pi = 0.85;
  float delta = 1 - pi;
  // Local Variable used to check if this thread's subvector
  // has converged.
  bool hasConverged = false;

  while(1)
  {
    if (threadId == 0)
      sum = 0;
    // Sync all threads before starting the calculations.
    pthread_barrier_wait(&barrier);

    // Swap the values of the previous and the current pagerank vector.
    float* tmpPtr = z;
    z = x;
    x = tmpPtr;

    if (threadId == 0)
      pageRankIterNum++;

    float localSum = 0;
    // For each vertex of the thread's chunk
    // update the corresponding entry of the pagerank vector.
    for (int j = 0; j < chunk; j++)
    {
      // If there are no outbound links then add a small probability
      // that corresponds to the user performing a random jump to any
      // website in the graph.
      if (c[j + start] == 0)
        localSum = localSum + z[j + start] / N;
      float tempSum = 0;
      for(int i = 0; i < d[j + start]; i++)
      {
        tempSum += z[L[j][i]] / c[L[j][i]];
      }
      x[j + start] = tempSum;
    }

    pthread_spin_lock(&sumMutex);
    sum = sum + localSum;
    pthread_spin_unlock(&sumMutex);

    pthread_barrier_wait(&barrier);

    // Update the values of the pagerank vector.
    for(int i = 0; i < chunk; i++)
      x[i + start] = pi * (x[i + start] + sum) + delta * E[i];

    float maximum = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < chunk; ++i)
    {
      float absDiff = fabs(x[i + start] - z[i + start]);
      if (maximum < absDiff)
        maximum = absDiff;
    }

    if (maximum < 1e-6 && hasConverged == false)
    {
      hasConverged = true;
      pthread_spin_lock(&terminationLock);
      threadsFinished++;
      pthread_spin_unlock(&terminationLock);
    }

    pthread_barrier_wait(&barrier);
    if (threadsFinished == NUM_THREADS)
      break;
  }
  pthread_exit(static_cast<void*>(NULL));
}


void pthreadsPageRankPow(int N, int *c, std::vector<int> *L, float *x, float *z,
    int *d, float* E, int numThreads, int numEdges)
{
  NUM_THREADS = numThreads;
  // Initialize the the thread array.
  // TO DO(Vassilis Choutas) Make dynamic
  pthread_t* threads = new pthread_t[NUM_THREADS];
  if (threads == NULL)
  {
    std::cout << "Could not allocate memory for thread array. Exiting!" << std::endl;
    exit(-1);
  }

  // Declare the array containing the data for each thread.
  threadData* dataArray= new threadData[NUM_THREADS];
  if (dataArray == NULL)
  {
    std::cout << "Could not allocate memory for the thread data array. Exiting!" << std::endl;
    exit(-1);
  }

  pthread_spin_init(&sumMutex, 0);
  pthread_spin_init(&terminationLock, 0);

  pthread_barrier_init(&barrier, NULL, NUM_THREADS);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // int chunk = N / numThreads;
  int chunk = numEdges / numThreads;

  std::vector<int> startPoint;
  std::vector<int> endPoint;

  // Set the first node as the starting node.
  int start = 0;
  for (int i = 0; i < numThreads; ++i)
  {
    int j = start;
    int sum = 0;
    // Iterate until the current thread has enough edges or
    // the final node has been reached.
    while (sum < chunk && j < N)
    {
      // Add the number of edges.
      sum += c[j];
      // Check the next node.
      j++;
    }
    // Calculate the final element for thread #i
    int end = j - 1;
    // Append the starting and the ending point.
    startPoint.push_back(start);
    endPoint.push_back(end);
    // Update the starting point variable for the next thread.
    start = end + 1;
  }
  endPoint[endPoint.size() - 1] = N;

#ifdef DEBUG
  assert(endPoint.size() == numThreads);
#endif

  // Initialize the data that will be passed to each thread.
  for (int i = 0; i < numThreads; i++)
  {
    // dataArray[i].chunk = chunk;
    dataArray[i].chunk = endPoint[i] - startPoint[i] + 1;
    // dataArray[i].start = i * chunk;
    dataArray[i].start = startPoint[i];
    dataArray[i].L = &L[dataArray[i].start];
    dataArray[i].c = c;
    dataArray[i].d = d;
    dataArray[i].N = N;
    dataArray[i].x = x;
    dataArray[i].z = z;
    dataArray[i].E = E;
    dataArray[i].threadId = i;
  }

  // dataArray[numThreads - 1].chunk = N % numThreads + chunk;

  void* status;

  std::cout << "Starting PageRank Calculation" << std::endl;
  struct timeval startwtime, endwtime;
  gettimeofday(&startwtime, NULL);

  for(int i = 0; i < NUM_THREADS; i++)
  {
    int rcode = pthread_create(&threads[i], &attr, threadedPageRank, (void*) &dataArray[i]);
    if (rcode)
    {
      printf("ERROR; return code from pthread_create() is %d\n", rcode);
      exit(-1);
    }
  }

  for(int i = 0; i < NUM_THREADS; i++)
  {
    int rcode = pthread_join(threads[i], &status);
    if (rcode)
    {
      printf("ERROR; return code from pthread_join() is %d\n", rcode);
      exit(-1);
    }
  }

  gettimeofday(&endwtime, NULL);
  double pagerankTime = (double)((endwtime.tv_usec - startwtime.tv_usec)
      /1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

  pthread_attr_destroy(&attr);
  pthread_spin_destroy(&sumMutex);
  pthread_barrier_destroy(&barrier);
  pthread_spin_destroy(&terminationLock);

  std::cout << "[Parallel PageRank]: Iterations = <" << pageRankIterNum << ">" << std::endl;
  std::cout << "[Parallel PageRank]: Time = <" << pagerankTime << "> seconds." << std::endl;


  delete[] threads;
  delete[] dataArray;

  return;
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    std::cout << "Usage: " << argv[0] << " graphFile numThreads" << std::endl
              << " \trandomSurfProbFile(optional)" << std::endl;;
    return -1;
  }

  std::string line;
  std::ifstream graphFile;

  int numThreads = atoi(argv[2]);

  // Open the file containing the description of the graph.
  graphFile.open(argv[1]);
  if (!graphFile.is_open())
  {
    std::cout << "Could not open the graph file!" << std::endl;
    return -1;
  }

  std::string token;
  bool nodeNumFound = false;

  int numEdges = -1;
  int N = -1;

  // Parse the file line by line to get the number of nodes.
  while (getline(graphFile, line))
  {
    std::stringstream ss(line);
    // Check that the line is a comment line.
    if (line[0] == '#')
    {
      // Parse the two first tokens, in order to remove the # character
      // from the stream and get the first word.
      ss >> token;
      ss >> token;
      std::size_t posFound = token.find("Nodes");
      // If the word Nodes is in the current string then the next token is
      // the number of nodes in the graph.
      if (posFound != std::string::npos)
      {
        ss >> N;
        nodeNumFound = true;

        ss >> token;
        ss >> numEdges;
        break;
      }
    }
  }

  if (!nodeNumFound)
  {
    std::cout << "Could not read the number of nodes in the graph. Exiting!" << std::endl;
    return -1;
  }

  while (getline(graphFile, line))
  {
    std::stringstream ss(line);
    // Check that the line is not a comment line.
    if (line[0] != '#')
    {
      int N1, N2;
      ss >> N1;
      ss >> N2;
      if (N1 > N)
        N = N1;
      if (N2 > N)
        N = N2;
    }
  }


  // An array containing the degree(number of edges) of each vertex.
  int* degreeArray;
  // An array that contains the connections of each vertex in the graph.
  // e.g. The adjacency matrix of the graph.
  std::vector<int>* adjMat;

  degreeArray = new int[N];
  if (degreeArray == NULL)
  {
    std::cout << "Could not allocate memory for array of vertex degrees. Exiting!" << std::endl;
    exit(-1);
  }

  int* inboundDegArray = new int[N];
  if (inboundDegArray == NULL)
  {
    std::cout << "Could not allocate memory for array of vertex degrees. Exiting!" << std::endl;
    exit(-1);
  }

  adjMat = new std::vector<int>[N];
  if (adjMat == NULL)
  {
    std::cout << "Could not allocate memory for the Adjacency Matrix of the graph. Exiting!" << std::endl;
    exit(-1);
  }

  float* x = new float[N];
  float* z = new float[N];

  for(int i =0; i < N; i++)
  {
    x[i] = 1.0 / N;
    z[i] = 0;
  }

  // Initialize the degree array.
  for (int i = 0; i < N; ++i)
  {
    degreeArray[i] = 0;
    inboundDegArray[i] = 0;
  }

  // Reset the file to the start.
  graphFile.clear();
  graphFile.seekg(0, std::ios::beg);
  std::cout << "Parsing the Web Graph!" << std::endl;

  int counter = 0;
  while (getline(graphFile, line))
  {
    std::stringstream ss(line);
    // Check that the line is not a comment line.
    if (line[0] != '#')
    {
      int fromIdx;
      int toIdx;
      ss >> fromIdx;
      ss >> toIdx;
      counter++;
      adjMat[toIdx].push_back(fromIdx);
      degreeArray[fromIdx]++;
      inboundDegArray[toIdx]++;
    }
  }

#ifdef DEBUG
  assert(counter == numEdges && ("\nThe number of connections parsed is not equal to the number of edges!"));
#endif

  std::cout << "[Parallel PageRank]: Connections read = " << counter << std::endl;
  graphFile.close();

  float* E = new float[N];
  if (argc == 4)
  {
    std::ifstream probFile;
    probFile.open(argv[3]);
    if (!probFile.is_open())
    {
      std::cout << "Could not open text file!" << std::endl;
      return -1;
    }
    int i = 0;
    while (getline(probFile, line))
    {
      std::stringstream ss(line);
      // Check that the line is not a comment line.
      if (line[0] != '#')
      {
        float prob;
        ss >> prob;
        E[i] = prob;
        i++;
      }
    }
  }
  else
  {
    for (int i = 0; i < N; ++i)
      E[i] = 1.0f / N;
  }

  pthreadsPageRankPow(N, degreeArray, adjMat, x, z, inboundDegArray, E, numThreads, numEdges);

#ifdef DEBUG
  std::string inputFile(argv[1]);
  std::size_t pos = inputFile.find(".");
  testPageRankVector(N, x, inputFile.substr(0, pos));
#endif

  delete[] adjMat;
  delete[] degreeArray;
  delete[] inboundDegArray;
  delete[] x;
  delete[] z;
  return 0;
}
