#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct Order {
    std::string username;
    std::string side;  // Can be "Buy" or "Sell"
    std::string asset;
    int amount;
    int price;

    // Equality operator
    bool operator==(const Order &other) const;
};


std::ostream &operator<<(std::ostream &os, const Order &order);

struct Trade {
    std::string buyer_username;
    std::string seller_username;
    std::string asset;
    int amount;
    int price;
};

