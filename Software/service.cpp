#include "service.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <windows.h>

#define sleep(n) Sleep(n)

SandwichService::SandwichService() : context(1) {
    sandwiches = {
        "Broodje Mexicano", "Broodje Viandel", "Broodje Boulet",
        "Broodje Cervela", "Broodje Gezond", "Broodje Bakpao", "Bicky"
    };

    sandwichPrices = {
        {"Broodje Mexicano", 3.5}, {"Broodje Viandel", 3.0}, {"Broodje Boulet", 3.8},
        {"Broodje Cervela", 3.2}, {"Broodje Gezond", 4.0}, {"Broodje Bakpao", 2.8}, {"Bicky", 3.3}
    };
}

std::string SandwichService::getDayOfWeek() {
    time_t now = time(0);
    tm* localTime = localtime(&now);
    const char* days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    return days[localTime->tm_wday];
}

std::pair<std::string, int> SandwichService::getDailyDiscount() {
    std::string day = getDayOfWeek();
    if (dailyDiscountCache.find(day) == dailyDiscountCache.end()) {
        int index = rand() % sandwiches.size();
        int discount = 5 + rand() % 16;
        dailyDiscountCache[day] = { sandwiches[index], discount };
    }
    return dailyDiscountCache[day];
}

std::map<std::string, int> SandwichService::loadVotes(const std::string& filename) {
    std::map<std::string, int> votes;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        auto sep = line.find(":");
        if (sep != std::string::npos) {
            std::string name = line.substr(0, sep);
            int count = std::stoi(line.substr(sep + 1));
            votes[name] = count;
        }
    }
    return votes;
}

void SandwichService::saveVotes(const std::map<std::string, int>& votes, const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& [name, count] : votes) {
        file << name << ":" << count << "\n";
    }
}

void SandwichService::subscriberThread() {
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "Broodje?>", 9);

    while (true) {
        zmq::message_t msg;
        if (subscriber.recv(msg, zmq::recv_flags::none)) {
            std::string text(static_cast<char*>(msg.data()), msg.size());
            std::cout << "[Subscriber] Received: " << text << std::endl;

            zmq::socket_t pusher(context, ZMQ_PUSH);
            pusher.connect("tcp://benternet.pxl-ea-ict.be:24041");

            std::string response;
            auto votes = loadVotes("votes.txt");

            if (text.find("Broodje?>List") == 0) {
                auto [discountItem, discountPercent] = getDailyDiscount();
                response = "Broodje!>Sandwich List (Today's discount: " + discountItem + " - " +
                    std::to_string(discountPercent) + "% off)\n";
                for (size_t i = 0; i < sandwiches.size(); ++i) {
                    response += std::to_string(i) + ": " + sandwiches[i] + " (" +
                        std::to_string(sandwichPrices[sandwiches[i]]).substr(0, 4) + " Euro)\n";
                }
            }
            else if (text.find("Broodje?>Vote") == 0) {
                std::istringstream iss(text);
                std::string ignore, voteStr;
                iss >> ignore >> ignore >> voteStr;

                try {
                    int index = std::stoi(voteStr);
                    if (index >= 0 && index < sandwiches.size()) {
                        votes[sandwiches[index]]++;
                        saveVotes(votes, "votes.txt");
                        response = "Broodje!>Voted for: " + sandwiches[index];
                    }
                    else {
                        response = "Broodje!>Invalid sandwich number.";
                    }
                }
                catch (...) {
                    response = "Broodje!>Invalid vote format.";
                }
            }
            else if (text.find("Broodje?>Top") == 0) {
                std::vector<std::pair<std::string, int>> sortedVotes(votes.begin(), votes.end());
                std::sort(sortedVotes.begin(), sortedVotes.end(),
                    [](auto& a, auto& b) { return b.second < a.second; });
                response = "Broodje!>Top Voted Sandwiches:\n";
                for (auto& [name, count] : sortedVotes) {
                    response += name + ": " + std::to_string(count) + "\n";
                }
            }
            else if (text.find("Broodje?>Buy") == 0) {
                std::string name = text.substr(std::string("Broodje?>Buy ").size());
                if (sandwichPrices.find(name) != sandwichPrices.end()) {
                    double price = sandwichPrices[name];
                    auto [discountSandwich, discountPercent] = getDailyDiscount();
                    if (name == discountSandwich) {
                        double finalPrice = price - (price * discountPercent / 100.0);
                        response = "Broodje!>You bought '" + name + "' with " +
                            std::to_string(discountPercent) + "% discount! Final price: " +
                            std::to_string(finalPrice).substr(0, 4) + " Euro";
                    }
                    else {
                        response = "Broodje!>You bought '" + name + "' for " +
                            std::to_string(price) + " Euro";
                    }
                }
                else {
                    response = "Broodje!>Sandwich not found.";
                }
            }
            else {
                response = "Broodje!>Commands: \n Broodje?>List \n Broodje?>Buy [name] \n Broodje?>Top \n Broodje?>Vote [index]";
            }

            zmq::message_t reply(response.data(), response.size());
            pusher.send(reply, zmq::send_flags::none);
            std::cout << "[Subscriber] Sent: " << response << std::endl;
        }
        sleep(100);
    }
}

void SandwichService::subscriberThreadListen() {
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "Herb!>", 6);

    while (true) {
        zmq::message_t msg;
        if (subscriber.recv(msg, zmq::recv_flags::none)) {
            std::string text(static_cast<char*>(msg.data()), msg.size());
            std::cout << "[Subscriber] Received: " << text << std::endl;
        }
        sleep(100);
    }
}

void SandwichService::pusherThread() {
    zmq::socket_t pusher(context, ZMQ_PUSH);
    pusher.connect("tcp://benternet.pxl-ea-ict.be:24041");

    std::string input;
    while (true) {
        std::cout << "[Pusher] Enter message (or 'exit'): ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        std::cout << "Today is: " << getDayOfWeek() << std::endl;
        zmq::message_t message(input.data(), input.size());
        pusher.send(message, zmq::send_flags::none);
        std::cout << "[Pusher] Sent: " << input << std::endl;
    }
}

void SandwichService::run() {
    srand(time(0));
    std::thread sub1(&SandwichService::subscriberThread, this);
    std::thread push(&SandwichService::pusherThread, this);
    std::thread sub2(&SandwichService::subscriberThreadListen, this);

    sub2.join();
    sub1.join();
    push.detach();
}
