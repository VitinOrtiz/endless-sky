/* Bay.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Bay.h"

#include "../Command.h"

void Bay::Receive(const std::shared_ptr<BrokerMessage> message, const std::string &topic)
{
	if(topic == "Ship")
	{
		// Deploy
		if(message->Action() == "Deploy")
		{
			if(carrier && carrier->UUID().ToString() == message->Actor())
			{
				if(message->Target() == "including damaged")
					Deploy(true);
				else
					Deploy(false);
			}
		}
	}
}

void Bay::Deploy(bool includingDamaged) const
{
	if(fighter && (includingDamaged || fighter->Health() > .75) &&
		(!fighter->IsYours() || fighter->HasDeployOrder()))
		fighter->SetCommands(Command::DEPLOY);
}
