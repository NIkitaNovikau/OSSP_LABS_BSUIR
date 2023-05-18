#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <cstring>

#define SHM_SIZE 1024
#define MAX_MESSAGES 10

typedef struct {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char data[256];
} Message;

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

void error(const char* msg) {
    perror(msg);
    exit(1);
}

int main() {
    key_t key = ftok("child.cpp", 'R');
    if (key == -1)
        error("ftok failed");

    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
        error("shmget failed");

    Message* sharedMemory = (Message*)shmat(shmid, NULL, 0);
    if (sharedMemory == (void*)-1)
        error("shmat failed");

    int semid = semget(key, 2, IPC_CREAT | 0666);
    if (semid == -1)
        error("semget failed");

    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1)
        error("semctl failed");

    arg.val = 0;
    if (semctl(semid, 1, SETVAL, arg) == -1)
        error("semctl failed");

    int producersCount = 0;
    int consumersCount = 0;

    std::cout << "Parent process started." << std::endl;

    while (true) {
        std::string command;
        std::cin >> command;

        if (command == "+p") {
            if (producersCount >= MAX_MESSAGES) {
                std::cout << "Max producers count reached." << std::endl;
                continue;
            }

            pid_t pid = fork();
            if (pid == -1) {
                error("fork failed");
            } else if (pid == 0) {
                execl("./child", "child", nullptr);
                error("execl failed");
            }

            ++producersCount;
            std::cout << "Started producer " << pid << std::endl;
        } else if (command == "-p") {
            if (producersCount <= 0) {
                std::cout << "No producers running." << std::endl;
                continue;
            }
            
            --producersCount;
            std::cout << "Stopped a producer." << std::endl;
        } else if (command == "+c") {
            if (consumersCount >= MAX_MESSAGES) {
                std::cout << "Max consumers count reached." << std::endl;
                continue;
            }

            pid_t pid = fork();
            if (pid == -1) {
                error("fork failed");
            } else if (pid == 0) {
                execl("./child", "child", nullptr);
                error("execl failed");
            }

            ++consumersCount;
            std::cout << "Started consumer " << pid << std::endl;
            } else if (command == "-c") {
            if (consumersCount <= 0) {
            std::cout << "No consumers running." << std::endl;
            continue;
            }
        --consumersCount;
        std::cout << "Stopped a consumer." << std::endl;
    } else {
        std::cout << "Unknown command." << std::endl;
    }
}

shmdt(sharedMemory);
shmctl(shmid, IPC_RMID, NULL);
semctl(semid, 0, IPC_RMID, arg);
semctl(semid, 1, IPC_RMID, arg);

std::cout << "Parent process finished." << std::endl;

return 0;
}