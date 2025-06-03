#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <zmq.hpp>

class SandwichService {
private:
    zmq::context_t context;
    std::vector<std::string> sandwiches;
    std::map<std::string, double> sandwichPrices;
    std::map<std::string, std::pair<std::string, int>> dailyDiscountCache;

    std::string getDayOfWeek();
    std::pair<std::string, int> getDailyDiscount();
    std::map<std::string, int> loadVotes(const std::string& filename);
    void saveVotes(const std::map<std::string, int>& votes, const std::string& filename);

    void saveOrder(const std::string& user, const std::string& sandwich);
    std::map<std::string, std::vector<std::string>> loadOrders();

    void subscriberThread();
    void subscriberThreadListen();
    void pusherThread();

public:
    SandwichService();
    void run();
};

#endif // SERVICE_H
