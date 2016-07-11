#include "ProducerConsumerQueue.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <string>
#include <atomic>

struct Data
{
	Data() {}
	Data(const std::string& name, const std::string& user)
		: Name(name), User(user)
	{}

	std::string Name = "";
	std::string User = "";
};

void test_pcq()
{
	auto begin = std::chrono::high_resolution_clock::now();

	folly::ProducerConsumerQueue<Data> queue(100000);
	volatile bool isdone = false;

	// productor
	std::thread productor = std::thread([&] {
		for (int i = 0; i < 10000; ++i) {
			queue.write(std::to_string(i), std::to_string(i));
		}
		isdone = true;
	});

	// consumer
	std::atomic_int count = 0;
	std::thread consumer = std::thread([&] {
		while (!isdone || !queue.isEmpty()) {
			Data data;
			if (queue.read(data)) {
				count++;
			}
		}
	});

	productor.join();
	consumer.join();

	auto end = std::chrono::high_resolution_clock::now();
	std::cout 
		<< "folly::ProducerConsumerQueue consumered: " << count  
		<< " left: " << queue.sizeGuess()
		<< " cost: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "us " << std::endl;
}
