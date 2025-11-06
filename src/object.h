#pragma once
#include <string>
#include <iomanip>
#include <sstream>
using namespace std;

class Object {
private:
    std::string name;
    int quantity;
    double price;
    double stock_value;

public:
    Object() : name(""), quantity(0), price(0.0), stock_value(0.0) {} 
    Object(const std::string& n, int q, double p) : name(n), quantity(q), price(p), stock_value(0.0) {}

    std::string to_string() const {
        std::ostringstream oss;
        oss << "| " << std::left  << std::setw(30) << name
        << "| " << std::right << std::setw(9)  << quantity
        << " | " << std::setw(9) << price
        << " | " << std::setw(12) << stock_value
        << " |";        return oss.str();
    }
    void setStockValue(double v) { stock_value = v; }
    
    double getStockValue() const { return stock_value; }
    
    double computeStockValue() const {
        return quantity * price;
    }
};