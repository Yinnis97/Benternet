#include <iostream>
#include <zmq.hpp>
#include <string>
#include <windows.h>
#include <thread>
#define sleep(n)    Sleep(n)

// Subcriber Thread
void subscriberThread(zmq::context_t& context)
{
	zmq::socket_t subscriber(context, ZMQ_SUB);
	subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "Broodje?>", 9);

	while (true)
	{
		zmq::message_t received_data;
		std::string response = "";

		// Use non-blocking receive
		if (subscriber.recv(received_data, zmq::recv_flags::none))
		{
			// Parse Received data
			std::string msg(static_cast<char*>(received_data.data()), received_data.size());
			std::cout << "[Subscriber] Received: " << msg << std::endl;

			// Create separate PUSH socket for reply
			zmq::socket_t pusher(context, ZMQ_PUSH);
			pusher.connect("tcp://benternet.pxl-ea-ict.be:24041");

			// Generate Random msg as a reply
			int rndm = rand() % 5;
			switch (rndm)
			{
			case 0:
				response = "Broodje!>Broodje Mexicano";
				break;
			case 1:
				response = "Broodje!>Broodje Viandel";
				break;
			case 2:
				response = "Broodje!>Broodje Boulet";
				break;
			case 3:
				response = "Broodje!>Broodje Cervela";
				break;
			case 4:
				response = "Broodje!>Broodje Gezond";
				break;
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
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "Herbs?>", 7);

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