#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <map>


#include "useraccount.hpp"
#include "utility.hpp"

class Exchange {
  private:
    std::unordered_map<std::string, std::unordered_map<std::string, int>> portfolios;
    std::unordered_map<std::string, std::vector<Order>> openOrders;
    std::vector<Trade> tradeHistory;
    std::unordered_map<std::string, std::vector<Order>> filledOrders; 
    void UpdatePortfolioAfterTrade(const Trade& trade);

    std::vector<Order> openOrdersChron;
    std::vector<Order> filledOrdersChron;


 public:
  std::unordered_map<std::string, int>& getPortfolioAssets(const std::string& username);
  void MakeDeposit(const std::string &username, const std::string &asset,
                   int amount);
  void PrintUserPortfolios(std::ostream &os) const;
  bool MakeWithdrawal(const std::string &username, const std::string &asset,
                      int amount);
  bool AddOrder(const Order &order);
  void PrintUsersOrders(std::ostream &os) const;
  void PrintTradeHistory(std::ostream &os) const;
  void PrintBidAskSpread(std::ostream &os) const;
  
};