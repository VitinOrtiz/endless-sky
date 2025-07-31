/* Broker.h
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

#pragma once

#include "Message.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

class Subscriber;

class Broker
{
private:
	std::unordered_map<std::string, std::queue<std::shared_ptr<Message>>> topics;
	std::unordered_map<std::string, std::vector<std::shared_ptr<Subscriber>>> subscribers;
	std::mutex topicsMutex;
	std::condition_variable cv;
	std::atomic<bool> running;
	std::thread brokerThread;

	Broker() : running(false) {}
	~Broker() { Stop(); }
	Broker(const Broker &) = delete;
	Broker &operator=(const Broker &) = delete;
	void ProcessMessages();

public:
	static Broker &GetInstance();
	void Start();
	void Stop();
	void Publish(const std::string &topic, std::shared_ptr<Message> message);
	void Subscribe(const std::string &topic, std::shared_ptr<Subscriber> subscriber);
};

