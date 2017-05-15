#include <fstream>
#include <iostream>
#include "eigenmvn.h"
#include <cstdio> 
#include <vector>
#include <iostream>

#ifndef M_PI
#define M_PI REAL(3.1415926535897932384626433832795029)
#endif

using namespace std; 

int ShowPlot(const char * name);
void CreatePoint();
Tuple GetAverage(vector<Tuple> & points);
void MinusAverage(vector<Tuple> & points, vector<Tuple> & avePoints);

typedef struct Tuple
{
    float x;
    float y;
}Tuple;
/**
  Take a pair of un-correlated variances.
  Create a covariance matrix by correlating 
  them, sandwiching them in a rotation matrix.
*/
Eigen::Matrix2d genCovar(double v0,double v1,double theta)
{
  Eigen::Matrix2d rot = Eigen::Rotation2Dd(theta).matrix();
  return rot*Eigen::DiagonalMatrix<double,2,2>(v0,v1)*rot.transpose();
}

int main()
{
  vector<Tuple> points;
  vector<Tuple> avePoints;
  Tuple averagePoint;

  CreatePoint();
  ShowPlot("plot 'samples_solver.txt'\n");

  ReadPoint(points);
  averagePoint = GetAverage(points);
  MinusAverage(points, avePoints, averagePoint);
  
  return 0;
}

void MinusAverage(vector<Tuple> & points, vector<Tuple> & avePoints, Tuple average)
{
  for (size_t i; i < points.size(); i++)
  {
    Tuple temp;
    temp.x = points.at(i).x - average.x;
    temp.y = points.at(i).y - average.y;
    avePoints.push_back(temp);
  }
}

// 求全部点的平均值
Tuple GetAverage(vector<Tuple> & points)
{
  Tuple average;
  float xSum = 0.0;
  float ySum = 0.0;

  for (size_t i; i < points.size(), i++)
  {
    xSum += points.at(i).x;
    ySum += points.at(i).y;
  }

  average.x = xSum / points.size();
  average.y = ySum / points.size();

  return average;
}

void ReadPoint(vector<Tuple> & points)
{
  string line;
  ifstream input("samples_solver.txt");

  while (getline(input, line))
  {
    Tuple point;
    istringstream record(line);
    record >> point.x;
    record >> point.y;
    points.push_back(point);
  }

  for (int i = 0; i < points.size(); i++)
  {
    cout << "(" << points.at(i).x << ", " << points.at(i).y << ") ";
    if (i+1 %10 == 0)
      cout << endl;
  }

  input.close();
}

int ShowPlot(const char * name)
{
  FILE *fp = popen("gnuplot", "w");
  if (fp == NULL) 
    return -1; 

  cout << name << endl;
  fputs("set mouse\n", fp); 
  fputs(name, fp); 
  fflush(fp); 
  cin.get(); 
  pclose(fp); 
  return 0;
}

void CreatePoint()
{
  Eigen::Vector2d mean;
  Eigen::Matrix2d covar;
  mean << -1,0.5; // Set the mean
  // Create a covariance matrix
  // Much wider than it is tall
  // and rotated clockwise by a bit
  covar = genCovar(3,0.1,M_PI/5.0);

  // Create a bivariate gaussian distribution of doubles.
  // with our chosen mean and covariance
  const int dim = 2;
  Eigen::EigenMultivariateNormal<double> normX_solver(mean,covar);
  std::ofstream file_solver("samples_solver.txt");

  // Generate some samples and write them out to file
  // for plotting
  file_solver << normX_solver.samples(500).transpose() << std::endl;

  // same for Cholesky decomposition.
  covar = genCovar(3,0.1,M_PI/5.0);
  Eigen::EigenMultivariateNormal<double> normX_cholesk(mean,covar,true);
  std::ofstream file_cholesky("samples_cholesky.txt");
  file_cholesky << normX_cholesk.samples(500).transpose() << std::endl;

  file_solver.close();
  file_cholesky.close();

}