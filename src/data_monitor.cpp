#include "object.h"
#include <mutex>
#include <condition_variable>
using namespace std;

#define MAX_SIZE 10
class DataMonitor {
private:
    int front;
    int rear;
    int count;
    Object arr[MAX_SIZE];

    bool finished;
    mutex mtx;
    condition_variable not_full;
    condition_variable not_empty;
public:
    DataMonitor() : front(0), rear(0), count(0) {}

    void push(const Object& val) {
        unique_lock<mutex> lock(mtx);
        not_full.wait(lock, [this]{return count < MAX_SIZE;});
       
        arr[rear] = val;
        rear = (rear + 1) % MAX_SIZE;
        count++;

        not_empty.notify_one();
    }

    bool pop(Object &result) {
        unique_lock<mutex> lock(mtx);
        not_empty.wait(lock,[this]{return count > 0 || finished;});
        
        if (count == 0 && finished){
            return false;
        }

        result = arr[front];
        front = (front + 1) % MAX_SIZE;
        count--;

        lock.unlock();
        not_full.notify_one();
        return true;
    }

    void setFinished() {
        unique_lock<mutex> lock(mtx);
        finished = true;
        lock.unlock();
        not_empty.notify_all();
    }
};