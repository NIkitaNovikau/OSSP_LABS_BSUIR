#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string.h>
using namespace std;

int main(int argc, char* argv[], char* envp[]){
	cout << "Name: " << argv[0] << endl;
    cout << "PID: " << getpid() << endl;
    cout << "PPID: " << getppid() << endl;
    // Open and read variables file
    FILE *file = fopen("text.txt", "r");
    char var_name[256];
    while (fscanf(file, "%s", var_name) != EOF) {
	char *way=new char[2048];
		way=getenv(var_name);
		if(way==NULL)
		{
			cout << var_name<< "=" <<"NULL"<<endl;
		}
		else
		{
			cout << var_name<< "=" <<way<<endl;
		}
    }
    fclose(file);
    return 0;
}