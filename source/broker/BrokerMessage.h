/* BrokerMessage.h
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

#include <string>

class BrokerMessage
{
private:
	std::string actor;
	std::string action;
	std::string target;

public:
	BrokerMessage(BrokerMessage &msg) = default;
	BrokerMessage(std::string &actor, std::string &action, std::string &target) :
		actor(std::move(actor)), action(std::move(action)), target(std::move(target)) { }

	std::string Actor() const { return actor; }
	std::string Action() const { return action; }
	std::string Target() const { return target; }
};
