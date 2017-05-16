// reading a text file
#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;
int main ()
{
    char buffer[256];
    ifstream examplefile("test.txt");
    ofstream exampleoutfile("test-tmp.txt");
    if (! examplefile.is_open())
    {
        cout << "Error opening file"; exit (1);
    }
    while (!examplefile.eof())
    {
        examplefile.getline(buffer,100);
        cout<<buffer<< endl;
        exampleoutfile << buffer << " " << buffer << endl;
    }

    exampleoutfile.close();
    examplefile.close();
    return 0;
}
