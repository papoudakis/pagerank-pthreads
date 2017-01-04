#include "utils.h"

void testPageRankVector(int N, float* result, const std::string& inputFileName)
{
  std::string groundTruthFile(inputFileName + "_prob.txt");

  double s = 0;
  for(int i = 0; i < N; i++){
    s = s + result[i];
  }

  std::cout << std::endl << "[Page Rank Vector Sum]: " << s << std::endl;

  for(int i = 0; i < 10; i++)
    std::cout << std::fixed << std::setprecision(4) << result[i] << " ";
  std::cout << std::fixed << std::setprecision(9) << std::endl;

  std::ifstream probFile;
  probFile.open(groundTruthFile.c_str());
  if (!probFile.is_open())
  {
    std::cout << "Could not open probability file!" << std::endl;
    return;
  }

  std::string line;

  int i = 0;

  double mae = -1;
  while (getline(probFile, line))
  {
    std::stringstream ss(line);
    // Check that the line is not a comment line.
    if (line[0] != '#')
    {
      double trueProb;
      ss >> trueProb;
      double temp = result[i] - trueProb;
      if (temp > mae)
        mae = temp;
    }
  }

  std::ios::fmtflags f;
  f = std::cout.flags();

  std::cout << std::scientific << "[PageRank Serial Test]: Max Absolute Error = " << mae << std::endl;

  std::cout.flags(f);

  return;
}
