#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include "../include/json.hpp"
#include "data_monitor.cpp"
#include "results_monitor.cpp"

using namespace std;
using json = nlohmann::json;

void worker(DataMonitor& dm, ResultsMonitor& rm, int criteria) {
    Object obj;
    while (dm.pop(obj)) {
        double val = obj.computeStockValue();
        obj.setStockValue(val);
        if (val > criteria) {
            rm.insertSorted(obj);
        }
    }
}

void write(const vector<Object>& results, const string& filename) {
    ofstream fout(filename);
    double total_sum = 0.0;
    fout << "-----------------------------------------------------------------------\n";
    fout << "| " << left << setw(30) << "Item name"
         << "| " << right << setw(9) << "Quantity"
         << " | " << setw(9) << "Price"
         << " | " << setw(12) << "Stock value"
         << " |\n";
    fout << "------------------------------------------------------------------------\n";
    for (const auto& obj : results) {
        fout << obj.to_string() << endl;
    }
    fout << "------------------------------------------------------------------------\n";
    fout.close();
}

int main(){
    vector<string> files = {"data/IFF-3-7_PadelskasE_L1_dat_1.json", "data/IFF-3-7_PadelskasE_L1_dat_2.json", "data/IFF-3-7_PadelskasE_L1_dat_3.json",};
    vector<Object> objects;
    for(const auto& path : files){
        ifstream file(path);
        json data = json::parse(file);
        for (const auto &item : data) {

            std::string name = item["name"];
            int quantity = item["quantity"];
            double price = item["price"];
            objects.emplace_back(name, quantity, price);
        }
        cout << "Finished reading from " << path << endl; 
    }
    cout << "Loaded " << objects.size() << " objects." << endl;


    DataMonitor dm;
    ResultsMonitor rm;
    const int criteria = 500;
    const int WORKER_COUNT = 10;
    vector<thread> workers;
    for (int i = 0; i < WORKER_COUNT; i++) {
        workers.emplace_back(worker, ref(dm), ref(rm), ref(criteria));
    }

    for (const auto obj : objects){
        dm.push(obj);
    }
    dm.setFinished();

    for (auto& t : workers) t.join();

    auto result = rm.getItems();
    write(result, "IFF-3-7_PadelskasE_L1_rez.txt");

    return 0;
}