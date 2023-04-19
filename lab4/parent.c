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

// Размер буфера сообщений
const int bufferSize = 256;

// Структура для представления сообщения
struct Message {
    char data[bufferSize];
    int size;
    int hash;
};

// Циклический буфер для хранения сообщений
queue<Message*> messageQueue;
int messageCount = 0;
int messageExtracted = 0;
mutex queueMutex;
condition_variable queueNotEmpty;//это примитив синхронизации, предоставляемый стандартной библиотекой C++11, который позволяет потокам ждать, 
//пока определенное условие станет истинным, прежде чем продолжить. 
condition_variable queueNotFull;
const int queueSize = 10;
const int messageLimit = 100;
//свой алгоритм подсчета hash
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

// Функция процесса производителя
void producer() {

    srand(time(NULL));

    while (true) {

        // Генерируем сообщение
        Message* message = new Message();
        message->size = rand() % 257;

        // Если размер равен нулю или 256, установите соответствующее значение
        if (message->size == 0) {
            message->size = 256;
        } else if (message->size == 256) {//а остальное делается парралельно 
            message->size = 0;
        }

        // Генерируем данные для сообщения
        for (int i = 0; i < 10; i++) {
            message->data[i] = char(rand() % 100);
        }

        // Рассчитываем хеш для сообщения
        message->hash = calculateHash(message->data);
        /////////////////////////////////////////////////////
        // Ожидаем свободного места в очереди
        unique_lock<mutex> locker(queueMutex);
        while (messageQueue.size() == queueSize) {
            queueNotFull.wait(locker);
        }
       
        messageQueue.push(message);//это делается синхронно
        sleep(1);
        // Увеличиваем количество сообщений и выводим на стандартный вывод
        messageCount++;
        cout << "Producer: Added message " << messageCount << endl;
        // Сообщаем потребителям, что очередь не пуста
        queueNotEmpty.notify_one();
        /////////////////////////////////////////////////////
    }
}
    // Функция процесса-потребителя
    void consumer() {

    while (true) {
    /////////////////////////////////////////////////////
      unique_lock<mutex> locker(queueMutex);
    while (messageQueue.empty()) {
        queueNotEmpty.wait(locker);
    }

    // Извлечь сообщение из очереди
    Message* message = messageQueue.front();/это делается синхронно
    messageQueue.pop();

    // Снимаем блокировку с очереди
    locker.unlock();
    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////
    // Проверяем хэш сообщения
    int calculatedHash = calculateHash(message->data);
    if (message->hash != calculatedHash) {
        cout << "Consumer: Message hash check failed" << endl;
    }
    sleep(1);
    // Увеличить количество извлеченных сообщений и вывести их на стандартный вывод
    messageExtracted++;
    cout << "Consumer: Extracted message " << messageExtracted << endl;//а это парралельно
    cout << "Got message hash :" << message->hash << "\n" << "Size text: " << message->size<< "\n"
    << "Text : " << message->data << endl;
    // Удаляем сообщение и уведомляем производителей, что очередь не заполнена
    delete message;
    queueNotFull.notify_one();
    /////////////////////////////////////////////////////
    }
}


int main() 
{ 
thread producerThread(producer);//это класс, предоставляемый стандартной библиотекой C++11, который представляет один поток выполнения. 
thread consumerThread(consumer);

// Подождите, пока пользовательский ввод завершится...
cout << "Press Enter to quit" << endl;
cin.ignore();
// Уведомляем потоки о выходе
producerThread.detach();//это функция, предоставляемая стандартной библиотекой C++11, которая позволяет вам отсоединить поток от вызывающего потока,
// чтобы два потока могли продолжать выполняться независимо. 
consumerThread.detach();
    return 0;
}
