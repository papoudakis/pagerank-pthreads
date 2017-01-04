#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <limits>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

#include "utils.h"

/**
 * @brief Function used to find the maximum per element absolute difference
 * between 2 arrays.
 * @param arr1[float*] The pointer to the first array
 * @param arr2[float*] The pointer to the second array
 * @param N[int] The number of elements in each array.
 * @return float The maximum absolute per element difference of the 2 arrays.
 */
float arrayDiffMax(float* arr1, float* arr2, int N)
{
  float maximum = -std::numeric_limits<float>::infinity();
  int i;

  for (i = 0; i < N; ++i)
  {
    float absDiff = fabs(arr1[i] - arr2[i]);
    if (maximum < absDiff)
      maximum = absDiff;
  }
  return maximum;
}

void pagerankpow(int N, int* c, std::vector<int>* L, float* pageRankVec, float* E)
{
  int i, j;
  int cnt = 0;
  float sum = 0;
  float pi = 0.85;
  float delta = 1 - pi;

  // The array holding the probabilities of arriving at each site/node
  float* z = new float[N];
  if (z == NULL)
  {
    std::cout << "Could not allocate memory for secondary Page Rank Vector . Exiting!" << std::endl;
    exit(-1);
  }

  // Initialize the probabilities arrays.
  for(i = 0; i < N; i++)
  {
    pageRankVec[i] = 1.0 / N;
    z[i] = 0;
  }

  // The epsilon value used for checking whether the pagerank algorithm has converged.
  float e = 1;


  struct timeval startwtime, endwtime;
  gettimeofday (&startwtime, NULL);

  // Iterate until convergence
  while(e > 1e-6)
  {
    sum = 0;

    // Initialize the necessary vectors for the current iteration.
    for(i = 0; i < N; i++)
    {
      z[i] = pageRankVec[i];
      pageRankVec[i] = 0;
    }

    for(j = 0; j < N; j++)
    {
      if(c[j] == 0)
        sum = sum + z[j] / N;
      else
      {
        for(i = 0; i < c[j]; i++)
          pageRankVec[L[j][i]] = pageRankVec[L[j][i]] + z[j] / c[j];
      }
    }

    for(i = 0; i < N; i++)
    {
      pageRankVec[i] = pi * (pageRankVec[i] + sum) + delta * E[i];
    }
    // Increase the iteration count.
    cnt = cnt + 1;
    // Calculate the maximum change between the current and the
    // previous PageRank vector in order to decide if to terminate
    // the calculations.
    e = arrayDiffMax(pageRankVec, z, N);
  }

  gettimeofday (&endwtime, NULL);
  double pagerankTime = (double)((endwtime.tv_usec - startwtime.tv_usec)
      /1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

  std::cout << "[Serial PageRank]: Time =  <" <<  pagerankTime << "> seconds." << std::endl;

  std::cout << "[Serial PageRank]: Iterations = <" << cnt << ">" << std::endl;

  delete[] z;
  return;
}


int main(int argc, char** argv){

  // Parse the input arguments.
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " graphFile" << std::endl
              << " \trandomSurfProbFile(optional)" << std::endl;
    return -1;
  }

  // Read the graph file.
  std::string line;
  std::ifstream graphFile;

  // Open the file containing the description of the graph.
  graphFile.open(argv[1]);
  if (!graphFile.is_open())
  {
    std::cout << "Could not open text file!" << std::endl;
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
  adjMat = new std::vector<int>[N];
  if (adjMat == NULL)
  {
    std::cout << "Could not allocate memory for the Adjacency Matrix of the graph. Exiting!" << std::endl;
    exit(-1);
  }


  // Initialize the degree array.
  for (int i = 0; i < N; ++i)
    degreeArray[i] = 0;

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
      adjMat[fromIdx].push_back(toIdx);
      degreeArray[fromIdx]++;
    }
  }

  // The matrix with the random jump probabilities.
  float* E = new float[N];
  // Check if an extra argument was provided.
  if (argc == 3)
  {
    // Parse the file.
    std::ifstream probFile;
    probFile.open(argv[2]);
    if (!probFile.is_open())
    {
      std::cout << "Could not open text file!" << std::endl;
      return -1;
    }
    int i = 0;
    // Iterate line by line in order to read the probabilities.
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
    // If no extra argument was provided then just assign uniform
    // probabilities to each vertex.
    for (int i = 0; i < N; ++i)
      E[i] = 1.0f / N;
  }

#ifdef DEBUG
  assert(counter == numEdges && ("\nThe number of connections parsed is not equal to the number of edges!"));
#endif

  std::cout << "Connections read = " << counter << std::endl;
  graphFile.close();

  // The array of the probabilities for the next iteration.
  float* pageRankVec = new float[N];
  if (pageRankVec == NULL)
  {
    std::cout << "Could not allocate memory for PageRank Vector . Exiting!" << std::endl;
    exit(-1);
  }

  pagerankpow(N, degreeArray, adjMat, pageRankVec, E);

#ifdef DEBUG
  std::string inputFile(argv[1]);
  std::size_t pos = inputFile.find(".");
  testPageRankVector(N, pageRankVec, inputFile.substr(0, pos));
#endif


  delete[] pageRankVec;
  delete[] degreeArray;
  delete[] adjMat;
  return 0;
}

