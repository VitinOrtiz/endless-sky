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

#include <thread>

using namespace std;



static shared_ptr<Broker> Broker::getInstance()
{
	if(!instance)
		instance = std::make_shared<Broker>();
	return instance;
}



void Broker::Subscribe(const string &topic, shared_ptr<Subscriber> subscriber)
{
	lock_guard<mutex> lock(mutex_);
	subscribers[topic].push_back(subscriber);
}



void Broker::Publish(const string &topic, shared_ptr<Message> message)
{
	{
		lock_guard<mutex> lock(mutex_);
		topics[topic].push(message);
	}
	cond_.notify_all();
}



void Broker::ProcessMessages()
{
	unique_lock<mutex> lock(mutex_);
	while(true)
	{
		cond_.wait(lock, [this]
			{
				for(const auto &topic : topics)
					if(!topic.second.empty())
						return true;
				return false;
			});
		for(auto &topic : topics)
		{
			string topic_name = topic.first;
			queue<shared_ptr<Message>> &q = topic.second;
			while(!q.empty())
			{
				shared_ptr<Message> message = q.front();
				q.pop();
				vector<shared_ptr<Subscriber>> &subs = subscribers[topic_name];
				for(auto &sub : subs)
					sub->Receive(message, topic_name);
			}
		}
	}
}
