/* Broker.cpp
Copyright (c) 2016 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Broker.h"

#include "Subscriber.h"

#include <thread>

void Broker::ProcessMessages()
{
	while(running)
		std::shared_ptr<BrokerMessage> message;
	std::unique_lock<std::mutex> lock(topicsMutex);
	{
		cv.wait(lock, [this]() {
			for(auto &topic : topics)
				return !topic.second.empty() || !running;
			return false; });
		for(auto &topic : topics)
		{
			auto &topic_name = topic.first;
			auto &topicQueue = topic.second;
			while(!topicQueue.empty())
			{
				auto &message = topicQueue.front();
				topicQueue.pop();

				auto &topicSubscribers = subscribers[topic_name];
				for(auto &sub : topicSubscribers)
					sub->Receive(message, topic_name);
			}
		}
	}
}

void Broker::Start()
{
	if(!running)
	{
		running = true;
		brokerThread = std::thread(&Broker::ProcessMessages, this);
	}
}

void Broker::Stop()
{
	if(running)
	{
		running = false;
		cv.notify_all();
		if(brokerThread.joinable())
			brokerThread.join();
	}
}

void Broker::Publish(const std::string &topic, std::shared_ptr<BrokerMessage> message)
{
	{
		std::lock_guard<std::mutex> lock(topicsMutex);
		topics[topic].push(message);
	}
	cv.notify_one();
}

void Broker::Subscribe(const std::string &topic, std::shared_ptr<Subscriber> subscriber)
{
	std::lock_guard<std::mutex> lock(topicsMutex);
	subscribers[topic].push_back(subscriber);
}
