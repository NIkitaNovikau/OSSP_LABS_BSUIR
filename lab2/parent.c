
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <cstdio>

using namespace std;


void output_environment_variables()
{

    extern char **environ;
    int i = 0;
    while (environ[i] != NULL)
    {
        cout << environ[i] << endl;
        i++;
    }
}


void generate_child_process(char* child_program_location, int& child_sequence_number)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // child процесс 
        char child_program_name[20];
        sprintf(child_program_name, "child_%02d", child_sequence_number);
        char* argv[] = {child_program_name, NULL};
        execve(child_program_location, argv, environ);
    }
    else if (pid > 0)
    {
        // parent процесс 
        child_sequence_number++;
    }
    else
    {

        cerr << "Failed to fork." << endl;
    }
}

void generate_child_process_env(char** env, int& child_sequence_number)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        char child_program_name[20];
        sprintf(child_program_name, "child_%02d", child_sequence_number);
        char* argv[] = {child_program_name, NULL};
        char* envp[] = {NULL};
        int env_count = 0;
        while (env[env_count] != NULL)
        {
            env_count++;
        }
        envp[env_count] = NULL;
        for (int i = 0; i < env_count; i++)
        {
            envp[i] = env[i];
        }
        execve(child_program_name, argv, envp);
    }
    else if (pid > 0)
    {
        child_sequence_number++;
    }
    else
    {
        cerr << "Failed to fork." << endl;
    }
}

void generate_child_process_environ(int& child_sequence_number)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        char child_program_name[20];
        sprintf(child_program_name, "child_%02d", child_sequence_number);
        char* argv[] = {child_program_name, NULL};
        char* envp[] = {NULL};
        int env_count = 0;
        while (environ[env_count] != NULL)
        {
            env_count++;
        }
        envp[env_count] = NULL;
        for (int i = 0; i < env_count; i++)
        {
            envp[i] = environ[i];
        }
        execve(child_program_name, argv, envp);
    }
    else if (pid > 0)
    {
        child_sequence_number++;
    }
    else
    {
        cerr << "Failed to fork." << endl;
    }
}

void output_required_environment_variables(char* filename="text.txt")
{
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        cerr << "Failed to open file." << endl;
        return;
    }

    char buffer[100];
    while (fgets(buffer, sizeof(buffer), file))
    {
        buffer[strcspn(buffer, "\r\n")] = 0;
        char* value = getenv(buffer);
        if (value != NULL)
        {
            cout << buffer << "=" << value << endl;
        }
        else if (value == NULL)
        {
            cout << buffer << "=" << "NULL" << endl;
        }
    }

fclose(file);
}


int main(int argc, char** argv, char** env)
{
    setenv("CHILD_PATH", "/home/kali/OSISP/lab2/child", 0);

    int child_sequence_number = 0;

    output_environment_variables();

    char input;
    while (true)
    {
        cin >> input;
        if (input == '+')
        {
            generate_child_process(getenv("CHILD_PATH"), child_sequence_number);
        }
        else if (input == '*')
        {
            output_required_environment_variables();
            generate_child_process_env(env, child_sequence_number);
        }
        else if (input == '&')
        {
            output_required_environment_variables();
            generate_child_process_environ(child_sequence_number);
        }
        else if (input == 'q')
        {
            break;
        }
    }

    // Terminate execution of parent process
    return 0;
}
