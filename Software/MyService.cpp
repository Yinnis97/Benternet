#include <iostream>
#include <zmq.hpp>
#include <string>
#include <windows.h>
#include <thread>
#include <fstream>
#include <map>
#include <sstream>
#include <ctime>
#define sleep(n)    Sleep(n)


std::string getDayOfWeek() {
    time_t now = time(0);
    tm* localTime = localtime(&now);

    int dayOfWeek = localTime->tm_wday; // 0 = Sunday, 1 = Monday, ..., 6 = Saturday

    const char* days[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };

    return days[dayOfWeek];
}

// Sandwich list
std::vector<std::string> sandwiches = {
    "Broodje Mexicano",
    "Broodje Viandel",
    "Broodje Boulet",
    "Broodje Cervela",
    "Broodje Gezond",
    "Broodje Bakpao",
    "Bicky"
};

std::map<std::string, double> sandwichPrices = {
    {"Broodje Mexicano", 3.5},
    {"Broodje Viandel", 3.0},
    {"Broodje Boulet", 3.8},
    {"Broodje Cervela", 3.2},
    {"Broodje Gezond", 4.0},
    {"Broodje Bakpao", 2.8},
    {"Bicky", 3.3}
};

// Generate daily discount
std::pair<std::string, int> getDailyDiscount() {
    std::string day = getDayOfWeek();
    static std::map<std::string, std::pair<std::string, int>> dailyDiscountCache;

    if (dailyDiscountCache.find(day) == dailyDiscountCache.end()) {
        int discountIndex = rand() % sandwiches.size();
        int discountPercent = 5 + rand() % 16; // [5, 20]
        dailyDiscountCache[day] = { sandwiches[discountIndex], discountPercent };
    }
    return dailyDiscountCache[day];
}

// Load votes from file
std::map<std::string, int> loadVotes(const std::string& filename)
{
    std::map<std::string, int> votes;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line))
    {
        auto sep = line.find(":");
        if (sep != std::string::npos) 
        {
            std::string name = line.substr(0, sep);
            int count = std::stoi(line.substr(sep + 1));
            votes[name] = count;
        }
    }
    return votes;
}

// Save votes to file
void saveVotes(const std::map<std::string, int>& votes, const std::string& filename) 
{
    std::ofstream file(filename);
    for (const auto& [name, count] : votes) {
        file << name << ":" << count << "\n";
    }
}

void subscriberThread(zmq::context_t& context)
{
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "Broodje?>", 9);

    while (true) {
        zmq::message_t received_data;
        std::string response = "";

        if (subscriber.recv(received_data, zmq::recv_flags::none)) {
            std::string msg(static_cast<char*>(received_data.data()), received_data.size());
            std::cout << "[Subscriber] Received: " << msg << std::endl;

            // Create reply socket
            zmq::socket_t pusher(context, ZMQ_PUSH);
            pusher.connect("tcp://benternet.pxl-ea-ict.be:24041");

            // Handle commands
            std::map<std::string, int> votes = loadVotes("votes.txt");

            if (msg.find("Broodje?>List") == 0) 
            {
                auto [discountSandwich, discountPercent] = getDailyDiscount();
                response = "Broodje!>Sandwich List (Today's discount: " + discountSandwich + " - " + std::to_string(discountPercent) + "% off)\n";

                for (size_t i = 0; i < sandwiches.size(); ++i) 
                {
                    std::string sandwich = sandwiches[i];
                    double price = sandwichPrices[sandwich];
                    response += std::to_string(i) + ": " + sandwich + " (" + std::to_string(price).substr(0, 4) + " Euro)\n";
                }
            }
            else if (msg.find("Broodje?>Vote") == 0)
            {
                std::istringstream iss(msg);
                std::string cmd, voteStr;
                iss >> cmd >> voteStr >> voteStr; // Skip "Broodje?>" and "Vote"

                try 
                {
                    int voteIndex = std::stoi(voteStr);
                    if (voteIndex >= 0 && voteIndex < sandwiches.size()) 
                    {
                        std::string sandwich = sandwiches[voteIndex];
                        votes[sandwich]++;
                        saveVotes(votes, "votes.txt");
                        response = "Broodje!>Voted for: " + sandwich;
                    }
                    else {
                        response = "Broodje!>Invalid sandwich number.";
                    }
                }
                catch (...) 
                {
                    response = "Broodje!>Invalid vote format.";
                }
            }
            else if (msg.find("Broodje?>Top") == 0) 
            {
                std::vector<std::pair<std::string, int>> sortedVotes(votes.begin(), votes.end());
                std::sort(sortedVotes.begin(), sortedVotes.end(),
                    [](auto& a, auto& b) { return b.second < a.second; });

                response = "Broodje!>Top Voted Sandwiches:\n";
                for (auto& [name, count] : sortedVotes) 
                {
                    response += name + ": " + std::to_string(count) + "\n";
                }
            }
            else if (msg.find("Broodje?>Buy") == 0) 
            {
                std::string name = msg.substr(std::string("Broodje?>Buy ").size());
                auto it = sandwichPrices.find(name);

                if (it != sandwichPrices.end()) 
                {
                    double price = it->second;
                    auto [discountSandwich, discountPercent] = getDailyDiscount();

                    if (name == discountSandwich) 
                    {
                        double discountAmount = price * discountPercent / 100.0;
                        double finalPrice = price - discountAmount;
                        response = "Broodje!>You bought '" + name + "' with " + std::to_string(discountPercent) +
                            "% discount! Final price: " + std::to_string(finalPrice).substr(0, 4) + " Euro";
                    }
                    else 
                    {
                        response = "Broodje!>You bought '" + name + "' for " + std::to_string(price) + " Euro";
                    }
                }
                else 
                {
                    response = "Broodje!>Sandwich not found.";
                }
            }
            else 
            {
                response = "Broodje!>Use the following commands : \n Broodje?>List : Show List \n Broodje?>Buy Broodje name : Buy a sandwich \n Broodje?>Top : Show the top voted sandwiches \n Broodje?>Vote number : Vote for a certain sandwich";
            }
            // Send reply
            zmq::message_t reply(response.data(), response.size());
            pusher.send(reply, zmq::send_flags::none);
            std::cout << "[Subscriber] Sent response: " << response << std::endl;
        }

        sleep(100);
    }
}


// Listen Thread
void subscriberThreadListen(zmq::context_t& context)
{
	zmq::socket_t subscriber(context, ZMQ_SUB);
	subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "beer!>", 6);

	while (true)
	{
		zmq::message_t received_data;

		// Use non-blocking receive
		if (subscriber.recv(received_data, zmq::recv_flags::none))
		{
			// Parse Received data
			std::string msg(static_cast<char*>(received_data.data()), received_data.size());
			std::cout << "[Subscriber] Received: " << msg << std::endl;

		}
		sleep(100);
	}
}

// Pusher Thread
void pusherThread(zmq::context_t& context)
{
	zmq::socket_t pusher(context, ZMQ_PUSH);
	pusher.connect("tcp://benternet.pxl-ea-ict.be:24041");

	std::string input;

	while (true)
	{
		std::cout << "[Pusher] Enter message to send (or type 'exit' to quit): ";
		std::getline(std::cin, input);

		if (input == "exit")
			break;
        std::string day = getDayOfWeek();
        std::cout << "Today is: " << day << std::endl;

		zmq::message_t message(input.data(), input.size());
		pusher.send(message, zmq::send_flags::none);
		std::cout << "[Pusher] Sent: " << input << std::endl;
	}
}

// Main
int main( void ) 
{ 
	srand(time(0));
	zmq::context_t context(1);

	// Start the threads
	std::thread subThread(subscriberThread, std::ref(context));
	std::thread pushThread(pusherThread, std::ref(context));
	std::thread subThreadListen(subscriberThreadListen, std::ref(context));
	
	subThreadListen.join();
	subThread.join();
	pushThread.detach();
	

	return 0; 
}