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

#include "Subscriber.h"
#include "Publisher.h"

#include <unordered_map>
#include <queue>
#include <vector>
#include <condition_variable>
#include <mutex>

class Broker
{
public:
	class Message
	{
	private:
		Publisher *publisher;
		std::string text;
	public:
		Message(Message &msg) = default;
		Message(Publisher &pub, std::string txt) : publisher(&pub), text(txt) {}
		Publisher *Publisher() const { return publisher; }
		std::string Text() const { return text; }
	};
private:
	unordered_map<std::string, queue<Broker::Message *>> topics;
	unordered_map<std::string, vector<Subscriber *>> subscribers;
	mutex mutex_;
	condition_variable cond_;
public:
	void Subscribe(const std::string &topic, Subscriber *subscriber);
	void Publish(const std::string &topic, Broker::Message *message);
	void ProcessEvents();
};

