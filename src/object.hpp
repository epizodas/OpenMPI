#pragma once
#include <cstring>
#include <iomanip>
#include <sstream>
using namespace std;

class Object {
private:
  char name[64];
  int quantity;
  double price;
  double stock_value;

public:
  Object() : quantity(0), price(0.0), stock_value(0.0) {
    name[0] = '\0';
  }

  Object(const std::string &n, int q, double p)
      : quantity(q), price(p), stock_value(0.0) {
        strncpy(name, n.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
  }

  void computeStockValue() { stock_value = quantity * price; }
  double getStockValue() const { return stock_value; }
  string getName() const { return string(name); }

  string to_string() const {
    ostringstream oss;
    oss << "| " << std::left << std::setw(30) << name << " | " << std::right
        << std::setw(9) << quantity << " | " << std::setw(9) << price << " | "
        << std::setw(12) << stock_value << " |";
    return oss.str();
  }
};