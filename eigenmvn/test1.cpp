#include <fstream>
#include "sstream"
#include "vector"
#include <iostream>

using namespace std;

typedef struct Tuple
{
    float x;
    float y;
}Tuple;

Tuple GetAverage(vector<Tuple> & points);
void MinusAverage(vector<Tuple> & points, vector<Tuple> & avePoints, Tuple average);
void Covariance(vector<Tuple> & points);

int main(void)
{
    vector<Tuple> points;
    string line;
    Tuple temp;

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

    cout << "points.size:" << points.size() << endl;
    
    Covariance(points);
}

// 求全部点的平均值
Tuple GetAverage(vector<Tuple> & points)
{
  Tuple average;
  float xSum = 0.0;
  float ySum = 0.0;

  for (int i; i < points.size(); i++)
  {
    xSum += points.at(i).x;
    ySum += points.at(i).y;
  }

  average.x = xSum / points.size();
  average.y = ySum / points.size();

  return average;
}

void MinusAverage(vector<Tuple> & points, vector<Tuple> & avePoints, Tuple average)
{
  for (int i; i < points.size(); i++)
  {
    Tuple temp;
    temp.x = points.at(i).x - average.x;
    temp.y = points.at(i).y - average.y;
    avePoints.push_back(temp);
  }

  cout << "Minussize: " << avePoints.size() << endl;
}

void Covariance(vector<Tuple> & points)
{
    Tuple temp;
    vector<Tuple> avePoints;
    float xx = 0.0;
    float yy = 0.0;
    float xy = 0.0;
    float tempx, tempy;
    temp = GetAverage(points);
    cout << "size: " << avePoints.size() << endl;
    MinusAverage(points, avePoints, temp);
    cout << "size: " << avePoints.size() << endl;
    for (int i = 0; i < avePoints.size(); i++)
    {
        tempx = avePoints.at(i).x;
        tempy = avePoints.at(i).y;
        cout << "avePoints.at(i).x: " << tempx << endl;
        cout << tempy << endl;
        xx += (tempx * tempx);
        xy += (tempx * tempy);
        yy += (tempy * tempy);
    }

    xx /= (avePoints.size() - 1);
    xy /= (avePoints.size() - 1);
    yy /= (avePoints.size() - 1);

    cout << "xx: " << xx << endl;
    cout << "xy: " << xy << endl;
    cout << "yy: " << yy << endl;
}