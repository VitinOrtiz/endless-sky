/* Subscriber.h
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

#include "BrokerMessage.h"

#include "Broker.h"

#include <memory>
#include <string>

class Subscriber : std::enable_shared_from_this<Subscriber>
{
public:
	virtual void Receive(const std::shared_ptr<BrokerMessage> message, const std::string &topic) {}
	void Subscribe(const std::string &topic)
	{
		Broker::GetInstance().Subscribe(topic, shared_from_this());
	}
};
