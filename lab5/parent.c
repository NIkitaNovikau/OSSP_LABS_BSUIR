#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <csignal>
#include <thread>

using namespace std;

const int bufferSize = 256;

struct Message {
    char data[bufferSize];
    int size;
    int hash;
};
//unique_lock vs lock_guard лучше unique_lock так как более гибким
// Размер буфера сообщений
// Циклический буфер для хранения сообщений
queue<Message*> messageQueue;
int messageCount = 0;
int messageExtracted = 0;
mutex queueMutex;
//это примитив синхронизации, предоставляемый стандартной библиотекой C++11, который позволяет потокам ждать, 
//пока определенное условие станет истинным, прежде чем продолжить. 
condition_variable queueNotEmpty;
condition_variable queueNotFull;
int queueSize = 5;

unsigned short calculateHash(char* hashed) {
    unsigned short crc = 0xFFFF;
    unsigned char i;
    unsigned char len = strlen(hashed);

    while (len--) {
        crc ^= *hashed++ << 8;

        for (i = 0; i < 8; i++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

void producer() {
    srand(time(NULL));

    while (true) {
        Message* message = new Message();
        message->size = rand() % 257;

        if (message->size == 0) {
            message->size = 256;
        } else if (message->size == 256) {
            message->size = 0;
        }

        for (int i = 0; i < 10; i++) {
            message->data[i] = char(rand() % 100);
        }

        message->hash = calculateHash(message->data);
        // Ожидаем свободного места в очереди
        unique_lock<mutex> locker(queueMutex);
        while (messageQueue.size() >= queueSize) {
            queueNotFull.wait(locker);
        }

        messageQueue.push(message);
        sleep(2);

        messageCount++;
        cout << "Producer: Added message " << messageCount << endl;

        locker.unlock();
        queueNotEmpty.notify_one();
    }
}

void consumer() {
    while (true) {
        unique_lock<mutex> locker(queueMutex);
        while (messageQueue.empty()) {
            queueNotEmpty.wait(locker);
        }

        Message* message = messageQueue.front();
        messageQueue.pop();
        // Снимаем блокировку с очереди
        locker.unlock();

        int calculatedHash = calculateHash(message->data);
        if (message->hash != calculatedHash) {
            cout << "Consumer: Message hash check failed" << endl;
        }
        sleep(2);

        messageExtracted++;
        cout << "Consumer: Extracted message " << messageExtracted << endl;
        cout << "Got message hash: " << message->hash << "\n"
             << "Size text: " << message->size << "\n"
             << "Text: " << message->data << endl;

        delete message;

        locker.lock();
        queueNotFull.notify_one();
    }
}

void signalHandler(int signum) {
    cout << "Received signal " << signum << ". Exiting..." << endl;
    exit(signum);
}

int main() {
    signal(SIGINT, signalHandler);  // Обработка сигнала прерывания (Ctrl+C)
    thread producerThread(producer);
    thread consumerThread(consumer);

    while (true) {
        char key = cin.get();
        if (key == '+') {
            cout << "Queue size : " << queueSize << endl;
            unique_lock<mutex> locker(queueMutex);
            queueSize++;
            cout << "///////////////////////////////////////////////////////////////////"<< endl;
            cout << "Queue size increased: " << queueSize << endl;
            cout << "///////////////////////////////////////////////////////////////////"<< endl;
            locker.unlock();
            queueNotFull.notify_all();
        } else if (key == '-') {
            cout << "Queue size : " << queueSize << endl;
            unique_lock<mutex> locker(queueMutex);
            if (queueSize > 0) {
                queueSize--;
                cout << "///////////////////////////////////////////////////////////////////"<< endl;
                cout << "Queue size decreased: " << queueSize << endl;
                cout << "///////////////////////////////////////////////////////////////////"<< endl;
                locker.unlock();
                queueNotFull.notify_all();
            }
        } else if (key == 'q') {
            cout << "Exiting..." << endl;
            raise(SIGINT);  // Генерация сигнала прерывания (Ctrl+C) для завершения программы
        }
    }

    producerThread.join();
    consumerThread.join();//используется для ожидания завершения выполнения потока

    return 0;
}
