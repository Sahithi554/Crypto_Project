#include "useraccount.hpp"
#include <iostream>
#include <stdexcept>

// Constructor to initialize the portfolio
Portfolio::Portfolio(double initialCash) : cash(initialCash) {}

// Check if the user has a specific asset in sufficient quantity
bool Portfolio::hasAsset(const std::string& asset, int quantity) const {
    auto it = assets.find(asset);
    return it != assets.end() && it->second >= quantity;
}

// Deduct a specific quantity of an asset
void Portfolio::deductAsset(const std::string& asset, int quantity) {
    if (!hasAsset(asset, quantity)) {
        throw std::runtime_error("Insufficient asset quantity!");
    }
    assets[asset] -= quantity;
    if (assets[asset] == 0) {
        assets.erase(asset);  // Remove asset entry if quantity is zero
    }
}

// Add a specific quantity of an asset
void Portfolio::addAsset(const std::string& asset, int quantity) {
    assets[asset] += quantity;
}

// Check if the user has enough cash
bool Portfolio::hasEnoughCash(double amount) const {
    return cash >= amount;
}

// Deduct a specific amount of cash
void Portfolio::deductCash(double amount) {
    if (!hasEnoughCash(amount)) {
        throw std::runtime_error("Insufficient cash!");
    }
    cash -= amount;
}

// Add cash to the portfolio
void Portfolio::addCash(double amount) {
    cash += amount;
}

// Print portfolio summary
void Portfolio::printSummary() const {
    std::cout << "Cash: $" << cash << "\nAssets:\n";
    for (const auto& [asset, quantity] : assets) {
        std::cout << "  " << asset << ": " << quantity << "\n";
    }
}

