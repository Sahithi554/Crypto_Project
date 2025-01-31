#pragma once
#pragma once
#include <string>
#include <unordered_map>

class Portfolio {
private:
    double cash;  // The amount of cash available
    std::unordered_map<std::string, int> assets;  // Assets owned, e.g., {"AAPL": 10}

public:
    Portfolio(double initialCash = 0.0);
    bool hasAsset(const std::string& asset, int quantity) const;
    void deductAsset(const std::string& asset, int quantity);
    void addAsset(const std::string& asset, int quantity);
    bool hasEnoughCash(double amount) const;
    void deductCash(double amount);
    void addCash(double amount);
    void printSummary() const;
    bool hasUSD(double amount) const;
    void deductUSD(double amount);
    void addUSD(double amount);
};

