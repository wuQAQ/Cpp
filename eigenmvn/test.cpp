#include <iostream> 
#include <cstdio> 
using namespace std; 

int main() 
{ 
  FILE *fp = popen("gnuplot", "w"); 
  if (fp == NULL) 
    return -1; 
  fputs("set mouse\n", fp); 
  fputs("plot 'samples_solver.txt'\n", fp); 
  fflush(fp); 
  cin.get(); 
  pclose(fp); 
  return 0; 
} 