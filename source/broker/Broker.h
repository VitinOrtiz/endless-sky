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
	static std::shared_ptr<Broker> instance;

	std::unordered_map<std::string, std::queue<std::shared_ptr<Message>>> topics;
	std::unordered_map<std::string, std::vector<std::shared_ptr<Subscriber>>> subscribers;
	std::mutex mutex_;
	std::condition_variable cond_;

	Broker() {}
public:
	Broker(Broker const &) = delete;
	void operator=(Broker const &) = delete;
	static std::shared_ptr<Broker> getInstance();
	void Subscribe(const std::string &topic, std::shared_ptr<Subscriber> subscriber);
	void Publish(const std::string &topic, std::shared_ptr<Message> message);
	void ProcessMessages();
};

