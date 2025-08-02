/* ShipEvent.cpp
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

#include "ShipEvent.h"

#include "Government.h"
#include "Ship.h"

using namespace std;

string EventToString(int type)
{
	string event;
	switch(type)
	{
	case ShipEvent::NONE:
		event = "none";
		break;
	case ShipEvent::ASSIST:
		event = "assist";
		break;
	case ShipEvent::SCAN_CARGO:
		event = "scan cargo";
		break;
	case ShipEvent::SCAN_OUTFITS:
		event = "scan outfits";
		break;
	case ShipEvent::PROVOKE:
		event = "provoke";
		break;
	case ShipEvent::DISABLE:
		event = "disable";
		break;
	case ShipEvent::BOARD:
		event = "board";
		break;
	case ShipEvent::CAPTURE:
		event = "capture";
		break;
	case ShipEvent::DESTROY:
		event = "destroy";
		break;
	case ShipEvent::ATROCITY:
		event = "atrocity";
		break;
	case ShipEvent::JUMP:
		event = "jump";
		break;
	case ShipEvent::ENCOUNTER:
		event = "encounter";
		break;
	}
	return event;
}

ShipEvent::ShipEvent(const Government *actor, const shared_ptr<Ship> &target, int type)
	: actorGovernment(actor), target(target), type(type)
{
	if(target)
		targetGovernment = target->GetGovernment();
	string event = EventToString(type);
	string message = actor->GetName() + " " + event + " " + target->Name();
	broker->Publish("ship", message);
}



ShipEvent::ShipEvent(const shared_ptr<Ship> &actor, const shared_ptr<Ship> &target, int type)
	: actor(actor), target(target), type(type)
{
	if(actor)
		actorGovernment = actor->GetGovernment();
	if(target)
		targetGovernment = target->GetGovernment();
	string event = EventToString(type);
	string message = actor->Name() + " " + event + " " + target->Name();
	broker->Publish("ship", message);
}



const shared_ptr<Ship> &ShipEvent::Actor() const
{
	return actor;
}



const Government *ShipEvent::ActorGovernment() const
{
	return actorGovernment;
}



const shared_ptr<Ship> &ShipEvent::Target() const
{
	return target;
}



const Government *ShipEvent::TargetGovernment() const
{
	return targetGovernment;
}



int ShipEvent::Type() const
{
	return type;
}
