#include <iostream>
#include <filesystem>
#include <stdlib.h>
#include <sys/wait.h> 
#include <string>
#include <sys/types.h>
#include <signal.h>
#include <limits.h>
#include <sstream>
#include <vector>       
#include <unistd.h>
#include <sys/stat.h>   //Использовать для межпроцессорного взаимодействия
#include <sys/mman.h>   //Для использования разделяемой памяти
#include <sys/ipc.h>	//
#include <sys/shm.h>	//

using namespace std; 

//Два варианта использования убийство процесса: 
//[1] -> SIGKILL - жестокое убийство процесса без возможности обработки сигнала!
//[2] -> SEGTERM - более мягкое убийство процесса, послание сигнала который в дальнейшем можно обрабатывать
//Будем использовать более мягкий вариант убийства процесса


//Общая память – это память, разделяемая между двумя или более процессами, которые устанавливаются с использованием общей памяти между всеми процессами.
//Этот тип памяти требует защиты друг от друга путем синхронизации доступа между всеми процессами.

//После испольтзовании функции fork() создается копия процесса, включая все его переменные и данные
//В результате, если вызвать push_back() в родительском процессе, после того как был создан дочерний процесс, то изменения не буду отображаться.
//Дочерний процесс всего лишь имеет копию адресной виртуальной памяти дочернего процесса(то есть адреса одинаковые, области памяти разные). 
//Поэтому мы будем использовать
// межпроцессорное взаимодействие или использование каналов передачи памяти!

extern char** environ;

int shm_open(const char* name, int oflag, mode_t mode); //Выделение разделяемой памяти

void exit_programm(int sig); 
int analysis_command(string command);


void exit_programm(int sig)
{
	char ch;
	while(true)
	{
		rewind(stdin);
		cout << endl;
		cout << "Точно вы хотите завершить программу?(Y/N)" << endl ;
		ch = getchar();
		if(ch == 'N')
		{
			break;
		}
		else if(ch == 'Y')
		{
			cout << "Программа завершена с кодом: " << sig << endl;
			exit(sig);
		}
		else 
		{
			cout << "Неверный вариант!" << endl;
			continue;
		}
	}
}


int analysis_command(string command)
{
	int a; 
	if(command.length() == 1)
	{
		if(command == "+"){	a = 1;}
		else if(command == "-"){ a = 2;}
		else if(command == "l") {a = 3;}
		else if(command == "k") {a = 4;}
		else if(command == "s") {a = 5;}
		else if(command == "g") {a = 6;}
		else if(command == "q") {a = 10;}
		else {a = 100;}
	}
	else
	{
		if(command.substr(0,2) == "s<"){return 7;}
		else if (command.substr(0,2) == "g<"){return 8;}
		else if (command.substr(0,2) == "p<") {return 9;}
	}
	return a;
}

void information_programm()
{
	cout << "[+] -> родительский процесс порождает дочерний." << endl
	 	 << "[-] -> удаление последнего дочернего процесса и вывода информации о осташихся." << endl
		 << "[l] -> вывод дочерних и ррдительских процессов." << endl
		 << "[k] -> удаление всех дочерних процессов и сообщение об завершении." << endl 
		 << "[s] -> запрет всем дочерним процессам выводить статистику." << endl
		 << "[g] -> разрешает всем дочерним процессам выводить статистику." << endl
		 << "[s<num>] -> запрещает С_<num> выводить статистику." << endl 
		 << "[g<num>] -> разрешает С_<num> выводить статистику." << endl
		 << "[p<num>] -> запрещает всем C_k вывод и запрашивает C_<num> вывести свою статистику. По истечению заданного времени (5 с, например), если не введен символ «g», разрешает всем C_k снова выводить статистику." << endl
		 << "[q] -> удаление всех дочерних процессов, сообщение об этом и завершение программы." << endl;
}


//Просмотреть использлование межпроцессорное взаимодействие
int main(int argc, char* argv[])
{ 
	cout << "Программа запущена, создан родительский процесс!" << endl;
	information_programm();

	signal(SIGINT, exit_programm);
	vector<int> counter;
	
	//Использование разделяемой памяти в одно целое число флаги(создание и доступ всех пользователей)
	int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0666);
	int* shared_mem = (int*)(shmat(shmid, NULL, 0));	
	//shmget() Поолучили ключ разделяемой памяти		  
	//shmat() присоединяет сегмент разделяемой памяти, идентифицированный shmid, к адресному пространству вызывающего процесса. 



	string command;
	while(true)
	{
		cout << "Введите команду для выбора действия: ";
		getline(cin, command, '\n');
		switch(analysis_command(command))
		{
			case 1:
			{
				string path = "/home/kali/OSISP/lab3/child";
				char* path_cstr = (char*)(path.c_str());
				pid_t child = fork();
				if(child ==0)
				{
					//Родительская область видимости не может присвоить значение другого процесса в родительскую переменную
				    *shared_mem = getpid();
					cout << "PID в разделяемой памяти [" << *shared_mem << "]" << endl;
					cout << "PID дочернего процесса добавлен в счетчик [" << getpid() << "]" << endl;
					char* argv[] = { path_cstr, NULL};
					execve(path_cstr, argv, NULL);
				}
				//Записываем значение из разделяемой памяти
				sleep(1); // Не лучший вариант использования, влияние на производительность
				cout << "Разделяемая память после выхода из области видимости -> " << *shared_mem << endl;
				counter.push_back(*shared_mem); 
				break;
			}
			case 2:
			{
				cout << "Сработала команда вызова убийства последнего дочернего процесса!" << endl;
				if(counter.size() == 0 )
				{
					cout << "Последнего дочернего процесса не существует!" << endl;
				}
				else
				{
			    int last_process = counter.at(counter.size()-1);
				kill(last_process, SIGKILL);
				//SIGKILL ‒ немедленное завершение программы
				//Сигнал SIGKILL используется для немедленного завершения программы. Его нельзя обрабаты-
				//вать или игнорировать, поэтому он всегда фатален.
				//Заблокировать сигнал SIGKILL невозможно.
				counter.erase(counter.begin() +counter.size() -1);
				}
				break;
			}
			case 3:	
			{
				cout << "Вывод все процессов программы!" << endl;
				cout << "[0] -> Родительский процесс: [" << getpid() << "]" << endl;
				if(counter.size() == 0)
				{
					cout << "[-] -> дочерние процессы отсутствуют." << endl;
				}
				else 
				{
					int a = 1;
					for(auto i: counter)
					{
						cout <<"[" << a << "] -> Дочерний процесс: [" << i << "]" << endl;
						++a; 
					}
				}
				break;
			}
			case 4:	
			{
				if(counter.size() == 0)
				{
					cout << "Нет дочерних процессов запущенных родительским процессом!" << endl;
				}
				else 
				{
					cout << "Режим удаления запущен!" << endl;
					for(int i = counter.size()-1; i > -1; --i)
					{
						int number = counter.at(i);
						kill(number, SIGKILL);
						counter.erase(counter.begin() + i);
					}
					cout << "Удаление всех дочерних процессов завершено!" << endl;
				}
				break;
			}
			case 5:	
			{
				cout << "Включен запрет вывода статистики всем дочерним процессам!" << endl;
				if(counter.size() == 0)
				{
					cout << "Дочерних процессов не существует!" << endl;
				}
				else 
				{
					for(int i = 0; i < counter.size(); ++i)
					{
						int pid = counter.at(i);
						kill(pid, SIGSTOP);
						//Сигнал SIGSTOP останавливает процесс.
						//Его нельзя обработать, игнорировать или заблокировать.
					}
				}
				cout << "Все дочерний процессы приостановлены!" << endl;
				break;
			}
			case 6:	
			{
				cout << "Подключение разешения на вывод дочерним процессам статистику!" << endl;
				if(counter.size() == 0)
				{
					cout << "Нет дочерних процесссов!" << endl;
				}
				else 
				{
					for(int i = 0; i < counter.size(); ++i )
					{
						int pid = counter.at(i);
						kill(pid, SIGCONT);
						//тот сигнал особый — он всегда заставляет процесс продолжать выполнение, если он останов-
						//лен, до того, как сигнал будет доставлен. По умолчанию больше ничего не делается.
						//Заблокировать сигнал SIGCONT невозможно.
					}
					cout << "Активирование включено успешно!" << endl;
				}
				break;
			}
			case 7:  
			{
				//короче сначала мы присваиваем строку number_pid_str
				//потом с помощью stringstream конвентим в число 
				//это все мы делаем чтобы прочитало pid
				string number_pid_str;
				int number_pid;
				number_pid_str = command.substr(2,5);
				stringstream buffer;
				buffer << number_pid_str;
				buffer >> number_pid;
				if(counter.size() == 0)
				{
					cout << "Родительский процесс не запускал дочерние процессы!" << endl;
				}
				else
				{
					for(int i = 0; i < counter.size(); ++ i)
					{
						if(counter.at(i) == number_pid)//at это функция которая берет и считывает определенный символ 
						{
							cout << "Поступил сигнал остановки процесса!" << endl;
							kill(number_pid, SIGSTOP);
							
						}
					}
				}
				break;
			}
			case 8:	
			{
				string number_pid_str;
				int number_pid;
				number_pid_str = command.substr(2,5);
				stringstream buffer;
				buffer << number_pid_str;
				buffer >> number_pid;
				if(counter.size() == 0)
				{
					cout << "Родительский процесс не запускал дочерние процессы!" << endl;
				}
				else
				{
					for(int i = 0; i < counter.size(); ++ i)
					{
						if(counter.at(i) == number_pid)
						{
							cout << "Поступил сигнал остановки процесса!" << endl;
							kill(number_pid, SIGCONT);
						}
					}
				}
				break;
			}
			case 9: 
			{
				if(counter.size() == 0)
				{
					cout << "Нет дочерних процессов, которые можно запретить!" << endl;
				}
				else
				{
				cout << "Основано на разъединени процессов!" << endl;
				for(int i = 0; i< counter.size(); ++i)
				{
					kill(counter.at(i),SIGSTOP);
				}
				pid_t test = fork();
				
				if(test == 0)
				{
					cout << "Номер процесса в случае сбоя: " << getpid() << endl;
					char t;
					alarm(5);
					t = getchar();
					if(pause() && t == NULL)
					{
						kill(getpid(), SIGKILL);
					}
					else if( t == 'g')
					{
						stringstream buffer;
						buffer << command.substr(2,5);
						int number_pid ;
						buffer >> number_pid;
						kill(number_pid, SIGCONT);
						kill(getpid(), SIGKILL);
					}
				}
				else
				{
					int status;
					waitpid(test, &status, 0);
					for(int i = 0; i < counter.size(); ++i)
					{
						kill(counter.at(i), SIGCONT);
					}
					cout << "Процесс обработки завершился!" << endl;
				}
				}
				break;
			}
			case 10:	
			{ 
				//Реализовать убийство всех дочерних процессов
				cout << "Программа успешно завершена c кодом 0!" << endl;
				int child_process;
				if(counter.size() == 0)
				{
					cout << "Дочерних процессов запущенных родительским процессом нет!" << endl;
				}
				else 
				{
					//Так же можно реализовать жесткое убийство и более мягкое!
					for(int i =0; i < counter.size(); ++i )
					{
						kill(counter.at(i), SIGTERM);
					}
				}
				exit(0);
				break;
			}
			default:
			{
				cout << "Введена неверная комманда, повторить комманду!" << endl;
				break;
			}
		}
	}

	shmdt(shared_mem);                         //Отсоединение процесса от разделяемой памяти
	shmctl(shmid, IPC_RMID, NULL);			   //Использование для удаления разделяемой памяти
	return 0;
}
