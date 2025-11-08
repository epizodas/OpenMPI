#pragma once
#include "object.hpp"
#include <iostream>
#include <vector>
using namespace std;
class Queue {
private:
  int front;
  int rear;
  int size;
  int max_size;
  vector<Object> arr;
  bool finished;

public:
    explicit Queue(int def = 10)
  : front(0), rear(0), size(0), max_size(def), arr(def), finished(false) {}

  void push(const Object &val) {
    if(isFull()){
        cerr << "Queue is full, cannot push new object: " << val.to_string() << endl;
        return;
    }
    arr[rear] = val;
    rear = (rear + 1) % max_size;
    size++;
  }

  Object pop() {
    if (isEmpty()) {
      Object obj;
      return obj;
    }
    Object result = arr[front];
    front = (front + 1) % max_size;
    size--;
    return result;
  }

  void insertSorted(const Object &val) {
    if (isFull()) return;
    int pos = 0;
    while (pos < size && arr[pos].getStockValue() > val.getStockValue()) {
      pos++;
    }
    for (int i = size; i > pos; --i) {
      arr[i] = arr[i - 1];
    }
    arr[pos] = val;
    size++;
  }

  void setFinished() { finished = true; }

  bool isEmpty() const { return size == 0; }

  bool isFull() const { return size == max_size; }

  int getSize() const { return size; }

  int getCapacity() const { return max_size; }
};