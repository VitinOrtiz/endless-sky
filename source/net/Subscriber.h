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

//#include "Broker.h"
#include "../ShipEvent.h"
#include "Broker.h"

#include <vector>

using namespace std;

class Subscriber
{
protected:
	Broker *broker;
public:
	Subscriber() = default;
	Subscriber(Broker &broker) : broker(&broker) {}
	void Subscribe(const string &topic)
	{
		broker->Subscribe(topic, this);
	}
	virtual void Receive(const Broker::Message *message, const string &topic);
};
