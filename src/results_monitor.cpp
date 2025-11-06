#include <vector>
#include "object.h"
#include <mutex>

using namespace std;

#define CAPACITY 100
class ResultsMonitor {
private:
    Object arr[CAPACITY];
    int count;
    mutex mtx;  
public:
    ResultsMonitor() : count(0) {}

    void insertSorted(const Object& val) {
        unique_lock<mutex> lock(mtx);
        int pos = 0;
        while (pos < count && arr[pos].getStockValue() > val.getStockValue()) {
            pos++;
        }

        for (int i = count; i > pos; --i) {
            arr[i] = arr[i - 1];
        }
        arr[pos] = val;
        count++;
    }

    vector<Object> getItems() {
        unique_lock<mutex> lock(mtx);
        vector<Object> res;
        for (int i = 0; i < count; ++i) {
            res.push_back(arr[i]);
        }
        return res;
    }
};
