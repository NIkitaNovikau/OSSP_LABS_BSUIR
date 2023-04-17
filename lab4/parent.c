#include <typeinfo>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <cstring>
#include<unistd.h>               
using namespace std;

// Size of the message buffer
const int bufferSize = 256;

// Structure to represent a message
struct Message {
    char data[bufferSize];
    int size;
    int hash;
};

// Circular buffer to hold messages
queue<Message*> messageQueue;
int messageCount = 0;
int messageExtracted = 0;
mutex queueMutex;
condition_variable queueNotEmpty;
condition_variable queueNotFull;
const int queueSize = 10;
const int messageLimit = 100;

   unsigned short calculateHash(char* hashed){
        unsigned short crc = 0xFFFF;
        unsigned char i;
        unsigned char len = strlen(hashed);
     
        while( len-- )
        {
            crc ^= *hashed++ << 8;
     
            for( i = 0; i < 8; i++ )
            crc = crc & 0x8000 ? ( crc << 1 ) ^ 0x1021 : crc << 1;
        }
        return crc;
    }

// Producer process function
void producer() {

    // Seed the random number generator
    srand(time(NULL));

    while (true) {

        // Generate a message
        Message* message = new Message();
        message->size = rand() % 257;

        // If size is zero or 256, set it to the appropriate value
        if (message->size == 0) {
            message->size = 256;
        } else if (message->size == 256) {
            message->size = 0;
        }

        // Generate data for the message
        for (int i = 0; i < 10; i++) {
            message->data[i] = char(rand() % 100);
        }

        // Calculate hash for the message
        message->hash = calculateHash(message->data);

        // Wait for space in the queue
        unique_lock<mutex> locker(queueMutex);
        while (messageQueue.size() == queueSize) {
            queueNotFull.wait(locker);
        }
        // Add the message to the queue
        messageQueue.push(message);
        sleep(1);
        // Increment message count and output to stdout
        messageCount++;
        cout << "Producer: Added message " << messageCount << endl;
        // Notify consumers that the queue is not empty
        queueNotEmpty.notify_one();
    }
}
    // Consumer process function
    void consumer() {

    while (true) {

      unique_lock<mutex> locker(queueMutex);
    while (messageQueue.empty()) {
        queueNotEmpty.wait(locker);
    }

    // Extract the message from the queue
    Message* message = messageQueue.front();
    messageQueue.pop();

    // Release the lock on the queue
    locker.unlock();

    // Check the hash of the message
    int calculatedHash = calculateHash(message->data);
    if (message->hash != calculatedHash) {
        cout << "Consumer: Message hash check failed" << endl;
    }
    sleep(1);
    // Increment message extracted count and output to stdout
    messageExtracted++;
    cout << "Consumer: Extracted message " << messageExtracted << endl;
    cout << "Got message hash :" << message->hash << "\n" << "Size text: " << message->size<< "\n"
    << "Text : " << message->data << endl;
    // Delete the message and notify producers that the queue is not full
    delete message;
    queueNotFull.notify_one();
    }
}


int main() 
{ 
thread producerThread(producer); 
thread consumerThread(consumer);

// Wait for user input to exit
cout << "Press Enter to quit" << endl;
cin.ignore();
// Notify threads to quit
producerThread.detach();
consumerThread.detach();
    return 0;
}