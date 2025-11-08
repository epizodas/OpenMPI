#pragma once
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>

#include "../include/json.hpp"
#include "object.hpp"
#include "queue.hpp"

using namespace std;

inline vector<Object> read() {
  vector<string> files = {
      "data/IFF-3-7_PadelskasE_L1_dat_1.json",
      "data/IFF-3-7_PadelskasE_L1_dat_2.json",
      "data/IFF-3-7_PadelskasE_L1_dat_3.json",
  };
  vector<Object> objects;
  for (const auto &path : files) {
    ifstream file(path);
    nlohmann::json data = nlohmann::json::parse(file);
    for (const auto &item : data) {
      string name = item["name"];
      int quantity = item["quantity"];
      double price = item["price"];
      objects.emplace_back(name, quantity, price);
    }
    cout << "Finished reading from " << path << endl;
  }
  cout << "Loaded " << objects.size() << " objects." << endl;
  return objects;
}

inline void write(Queue &results, const string &filename) {
  ofstream fout(filename);
  fout << "--------------------------------------------------------------------"
          "---\n";
  fout << "| " << left << setw(30) << "Item name"
       << "| " << right << setw(9) << "Quantity"
       << " | " << setw(9) << "Price"
       << " | " << setw(12) << "Stock value"
       << " |\n";
  fout << "--------------------------------------------------------------------"
          "----\n";
  while (!results.isEmpty()) {
    fout << results.pop().to_string() << endl;
  }
  fout << "--------------------------------------------------------------------"
          "----\n";
  fout.close();
}