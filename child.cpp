
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <ctime>
#include <random>

#define SHM_SIZE 1024

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

void generateMessage(Message* message) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sizeDistribution(1, 256);
    std::uniform_int_distribution<> dataDistribution(32, 126);

    message->type = 1;
    message->hash = 0;
    message->size = sizeDistribution(gen);
    for (int i = 0; i < message->size; ++i) {
        message->data[i] = dataDistribution(gen);
        message->hash += message->data[i];
    }
}

int main() {
    key_t key = ftok("child.cpp", 'R');
    if (key == -1)
        error("ftok failed");

    int shmid = shmget(key, SHM_SIZE, 0);
    if (shmid == -1)
        error("shmget failed");

    Message* sharedMemory = (Message*)shmat(shmid, NULL, 0);
    if (sharedMemory == (void*)-1)
        error("shmat failed");

    int semid = semget(key, 2, 0);
    if (semid == -1)
        error("semget failed");

    sembuf buf;
    buf.sem_flg = 0;

    std::cout << "Child process started." << std::endl;

    while (true) {
        buf.sem_num = 0;  // Semaphore for adding message
        buf.sem_op = -1; // Decrease semaphore value
        semop(semid, &buf, 1);

        Message* message = sharedMemory;

        generateMessage(message);

        buf.sem_num = 1;  // Semaphore for consuming message
        buf.sem_op = 1;  // Increase semaphore value
        semop(semid, &buf, 1);

        std::cout << "Produced a message. Total: " << (int)message->size << std::endl;

        sleep(1);
    }

    shmdt(sharedMemory);

    std::cout << "Child process finished." << std::endl;

    return 0;
}
