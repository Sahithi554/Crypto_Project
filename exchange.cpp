#include "exchange.hpp"
#include "useraccount.hpp"
#include <iomanip>
#include <set>
#include <algorithm>
#include "utility.hpp"      
#include <unordered_map>   
#include <map>             
#include <string>           
#include <iostream>

void Exchange::MakeDeposit(const std::string& username, const std::string& asset, int amount) {
    if (amount <= 0) {
        std::cerr << "Error: Deposit amount must be positive.\n";
        return;
    }

    portfolios[username][asset] += amount;
}

void Exchange::PrintTradeHistory(std::ostream& os) const {
    os << "Trade History (in chronological order):\n";
    for (Trade trade : tradeHistory) {
        os << trade.buyer_username << " Bought " << trade.amount << " of " << trade.asset << " From " << trade.seller_username << " for " << trade.price << " USD\n";
    }
}

void Exchange::PrintBidAskSpread(std::ostream &os) const {
    os << "Asset Bid Ask Spread (in alphabetical order):\n";

    std::vector<std::string> keys;
    for (auto const& key : openOrders) {
        keys.push_back(key.first);
    }
    std::sort(keys.begin(), keys.end());

    for (std::string key : keys) {
        std::string highestOpenBuyStr = "NA";
        std::string lowestOpenSellStr = "NA";
        int highOpenBuyCount = -1;
        int lowOpenSellCount = 99999999;

        for (auto openOrder : openOrdersChron) {
            if (openOrder.side == "Buy" && openOrder.asset == key) {
                if (openOrder.price > highOpenBuyCount) {
                    highOpenBuyCount = openOrder.price;
                }
            } else if (openOrder.side == "Sell" && openOrder.asset == key) {
                if (openOrder.price < lowOpenSellCount) {
                    lowOpenSellCount = openOrder.price;
                }
            }
        }

        if (highOpenBuyCount != -1) {
            highestOpenBuyStr = std::to_string(highOpenBuyCount);
        }

        if (lowOpenSellCount != 99999999) {
            lowestOpenSellStr = std::to_string(lowOpenSellCount);
        }

        os << key << ": Highest Open Buy = " << highestOpenBuyStr << " USD and Lowest Open Sell = " << lowestOpenSellStr << " USD\n";
    }
}

void Exchange::PrintUserPortfolios(std::ostream& os) const {
    std::map<std::string, std::map<std::string, int>> sortedPortfolios;

    for (const auto& [username, assets] : portfolios) {
        for (const auto& [asset, amount] : assets) {
            if (amount > 0) {
                sortedPortfolios[username][asset] = amount;
            }
        }
        if (sortedPortfolios[username].empty()) {
            sortedPortfolios[username] = {};
        }
    }

    os << "User Portfolios (in alphabetical order):\n";

    for (const auto& [username, assets] : sortedPortfolios) {
        os << username << "'s Portfolio: ";
        if (assets.empty()) {
            os << "\n"; 
            continue;
        }

        bool first = true;
        for (const auto& [asset, amount] : assets) {
            if (!first) {
                os << ", "; 
            }
            os << amount << " " << asset;
            first = false;
        }
        os << ", \n"; 
    }
}

bool Exchange::MakeWithdrawal(const std::string &username, const std::string &asset, int amount) {
    if (portfolios.find(username) == portfolios.end()) {
        return false; 
    }
    auto &userAssets = portfolios[username];
    if (userAssets.find(asset) == userAssets.end()) {
        return false; 
    }
    if (userAssets[asset] < amount) {
        return false; 
    }
    userAssets[asset] -= amount; 
    if (userAssets[asset] == 0) {
        userAssets.erase(asset);
    }
    return true; 
}

std::unordered_map<std::string, int>& Exchange::getPortfolioAssets(const std::string& username) {
    return portfolios[username];
}

void Exchange::UpdatePortfolioAfterTrade(const Trade& trade) {
    portfolios[trade.buyer_username][trade.asset] += trade.amount;
    portfolios[trade.buyer_username]["USD"] -= trade.amount * trade.price;  // Subtract USD for the purchase price
    portfolios[trade.seller_username][trade.asset] -= trade.amount;
    portfolios[trade.seller_username]["USD"] += trade.amount * trade.price;  // Add USD for the selling price
}

bool Exchange::AddOrder(const Order &order) {
    auto bestOrder = order;
    int salePrice;

    if (order.side == "Buy") {
        int totalCost = order.amount * order.price;
        if (portfolios[order.username]["USD"] < totalCost) {
            return false;
        }
        
        portfolios[order.username]["USD"] -= totalCost;
    } 
    else if (order.side == "Sell") {
        if (portfolios[order.username][order.asset] < order.amount) {
            return false;
        }

        portfolios[order.username][order.asset] -= order.amount;
    }

    if (order.side == "Buy") {
        salePrice = 99999999;
    } 
    else if (order.side == "Sell") {
        salePrice = -1;
    }

    openOrders[order.asset].push_back(order);

    openOrdersChron.push_back(order);

    if (openOrders[order.asset].size() == 0) {
        return true;
    }

    std::vector<Order> filteredOrders;
    for (auto openOrder : openOrdersChron) {
        if (order.side == "Buy" && openOrder.price > order.price) {
          continue;
        }

        if (order.side == "Sell" && openOrder.price < order.price) {
          continue;
        }

        if (order.side != openOrder.side && order.asset == openOrder.asset) {
            filteredOrders.push_back(openOrder);
        }
    }

    if (filteredOrders.size() == 0) {
        return true;
    }

    for (auto openOrder : filteredOrders) {
        if (order.side == "Buy") {
            if (openOrder.price < salePrice) {
                salePrice = openOrder.price;
                bestOrder = openOrder;
            }
        } else {
            if (openOrder.price > salePrice) {
                salePrice = openOrder.price;
                bestOrder = openOrder;
            }
        }
    }

    for (unsigned i = 0; i < openOrders[order.asset].size(); i++) {
        auto openOrder = openOrders[order.asset][i];

        if (openOrder.side != order.side) {

            int saleAmount = std::min(order.amount, openOrder.amount);
            int excess = std::max(order.amount, openOrder.amount) - saleAmount;
            
            if (order.side == "Buy" && openOrder.username == bestOrder.username && bestOrder.price == openOrder.price && openOrder.price <= order.price) {
                portfolios[order.username][order.asset] += saleAmount;
                portfolios[openOrder.username]["USD"] += saleAmount * order.price;

                tradeHistory.push_back(Trade {order.username, openOrder.username, order.asset, saleAmount, order.price});

            } else if (order.side == "Sell" && openOrder.username == bestOrder.username && bestOrder.price == openOrder.price && openOrder.price >= order.price) {
                portfolios[order.username]["USD"] += saleAmount * order.price;
                portfolios[openOrder.username][order.asset] += saleAmount;

                tradeHistory.push_back(Trade {openOrder.username, order.username, order.asset, saleAmount, order.price});

            } else {
                continue;
            }

            Order orderFill = {order.username, order.side, order.asset, saleAmount, order.price};
            Order openOrderFill = {openOrder.username, openOrder.side, openOrder.asset, saleAmount, order.price};

            filledOrders[order.asset].push_back(orderFill);
            filledOrders[order.asset].push_back(openOrderFill);

            filledOrdersChron.push_back(openOrderFill);
            filledOrdersChron.push_back(orderFill);

            if (openOrder.amount - saleAmount != 0) {
                openOrders[openOrder.asset][i] = Order {openOrder.username, openOrder.side, openOrder.asset, excess, openOrder.price};
                
                for (unsigned i = 0; i < openOrdersChron.size(); i++) {
                  if (openOrdersChron[i].username == openOrder.username && openOrdersChron[i].side == openOrder.side && openOrdersChron[i].asset == openOrder.asset && openOrdersChron[i].amount == openOrder.amount && openOrdersChron[i].price == openOrder.price) {
                    openOrdersChron[i] = Order {openOrder.username, openOrder.side, openOrder.asset, excess, openOrder.price};
                  }
                }
            } else {
                for (unsigned i = 0; i < openOrdersChron.size(); i++) {
                  if (openOrdersChron[i].username == openOrder.username && openOrdersChron[i].side == openOrder.side && openOrdersChron[i].asset == openOrder.asset && openOrdersChron[i].amount == openOrder.amount && openOrdersChron[i].price == openOrder.price) {
                    openOrdersChron.erase(openOrdersChron.begin() + i);
                  }
                }

                openOrders[openOrder.asset].erase(openOrders[openOrder.asset].begin() + i);
            }

            if (order.amount - saleAmount != 0) {
                openOrders[openOrder.asset].pop_back();
                for (unsigned i = 0; i < openOrdersChron.size(); i++) {
                  if (openOrdersChron[i].username == order.username && openOrdersChron[i].side == order.side && openOrdersChron[i].asset == order.asset && openOrdersChron[i].amount == order.amount && openOrdersChron[i].price == order.price) {
                    openOrdersChron.erase(openOrdersChron.begin() + i);
                  }
                }
                
                if (order.side == "Buy") {
                    int totalCost = excess * order.price;
                    portfolios[order.username]["USD"] += totalCost;
                } else if (order.side == "Sell") {

                    portfolios[order.username][order.asset] += excess;
                }

                AddOrder({order.username, order.side, order.asset, excess, order.price});
            } else {
                openOrders[order.asset].pop_back();
                for (unsigned i = 0; i < openOrdersChron.size(); i++) {
                  if (openOrdersChron[i].username == order.username && openOrdersChron[i].side == order.side && openOrdersChron[i].asset == order.asset && openOrdersChron[i].amount == order.amount && openOrdersChron[i].price == order.price) {
                    openOrdersChron.erase(openOrdersChron.begin() + i);
                  }
                }
            }
            
            return true;
        }
    }

    return true;
}

void Exchange::PrintUsersOrders(std::ostream &os) const {
    std::vector<std::string> usernames;
    for (const auto& [username, assets] : portfolios) {
        usernames.push_back(username);
    }
    std::sort(usernames.begin(), usernames.end());

    os << "Users Orders (in alphabetical order):\n";

    for (const auto& username : usernames) {
        for (std::string orderState : {"Open", "Filled"}) {
            os << username << "'s " << orderState << " Orders (in chronological order):\n";

            std::vector<Order> userOrders;
            if (orderState == "Open") {
                for (const auto& order : openOrdersChron) {
                    if (order.username == username) {
                        userOrders.push_back(order);
                    }
                }
            } else {
                for (const auto& order : filledOrdersChron) {
                    if (order.username == username) {
                        userOrders.push_back(order);
                    }
                }                
            }

            for (const auto& order : userOrders) {
                os << order.side << " " << order.amount << " " 
                << order.asset << " at " << order.price << " USD by " 
                << username << "\n";
            }

        }
    }
}