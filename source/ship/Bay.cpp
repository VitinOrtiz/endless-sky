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

#include "../CategoryList.h"
#include "../Command.h"
#include "../DataNode.h"
#include "../DataWriter.h"
#include "../Effect.h"
#include "../GameData.h"
#include "../Preferences.h"
#include "../Random.h"
#include "../Ship.h"

#include <cassert>

using namespace std;

namespace {
	const string FIGHTER_REPAIR = "Repair fighters in";
	const vector<string> BAY_SIDE = { "inside", "over", "under" };
	const vector<string> BAY_FACING = { "forward", "left", "right", "back" };
	const vector<Angle> BAY_ANGLE = { Angle(0.), Angle(-90.), Angle(90.), Angle(180.) };
}



void Bay::Load(const std::string &key, const DataNode &child)
{
	// Subscribe to "bay" channel
	broker->Subscribe("bay", this);
	// While the `drone` and `fighter` keywords are supported for backwards compatibility, the
	// standard format is `bay <ship-category>`, with the same signature for other values.
	category = "Fighter";
	int childOffset = 0;
	if(key == "drone")
		category = "Drone";
	else if(key == "bay")
	{
		category = child.Token(1);
		childOffset += 1;
	}
	double x = child.Value(1 + childOffset);
	double y = child.Value(2 + childOffset);
	position = Point(x, y);

	for(int i = 3 + childOffset; i < child.Size(); ++i)
	{
		for(unsigned j = 1; j < BAY_SIDE.size(); ++j)
			if(child.Token(i) == BAY_SIDE[j])
				side = j;
		for(unsigned j = 1; j < BAY_FACING.size(); ++j)
			if(child.Token(i) == BAY_FACING[j])
				facing = BAY_ANGLE[j];
	}
	if(child.HasChildren())
		for(const DataNode &grand : child)
		{
			const string &grandKey = grand.Token(0);
			bool grandHasValue = grand.Size() >= 2;
			// Load in the effect(s) to be displayed when the ship launches.
			if(grandKey == "launch effect" && grandHasValue)
			{
				int count = grand.Size() >= 3 ? static_cast<int>(grand.Value(2)) : 1;
				const Effect *e = GameData::Effects().Get(grand.Token(1));
				launchEffects.insert(launchEffects.end(), count, e);
			}
			else if(grandKey == "angle" && grandHasValue)
				facing = Angle(grand.Value(1));
			else
			{
				bool handled = false;
				for(unsigned i = 1; i < BAY_SIDE.size(); ++i)
					if(grandKey == BAY_SIDE[i])
					{
						side = i;
						handled = true;
					}
				for(unsigned i = 1; i < BAY_FACING.size(); ++i)
					if(grandKey == BAY_FACING[i])
					{
						facing = BAY_ANGLE[i];
						handled = true;
					}
				if(!handled)
					grand.PrintTrace("Skipping unrecognized attribute:");
			}
		}

}



void Bay::FinishLoading(bool isNewInstance)
{
	// Ensure that all defined bays are of a valid category. Remove and warn about any
	// invalid bays. Add a default "launch effect" to any remaining internal bays if
	// this ship is crewed (i.e. pressurized).
	string warning;
	const auto &bayCategories = GameData::GetCategory(CategoryType::BAY);
	if(!bayCategories.Contains(category))
	{
		warning += "Invalid bay category: " + category + "\n";
	}
	if(side == Bay::INSIDE && launchEffects.empty())
		launchEffects.emplace_back(GameData::Effects().Get("basic launch"));
}



void Bay::Save(DataWriter &out) const
{
	double x = 2. * position.X();
	double y = 2. * position.Y();

	out.Write("bay", category, x, y);

	if(!launchEffects.empty() || facing.Degrees() || side)
	{
		out.BeginChild();
		{
			if(facing.Degrees())
				out.Write("angle", facing.Degrees());
			if(side)
				out.Write(BAY_SIDE[side]);
			for(const Effect *effect : launchEffects)
				out.Write("launch effect", effect->Name());
		}
		out.EndChild();
	}
}



void Bay::Launch(vector<Visual> &visuals)
{
	bool ejecting = carrier->IsDestroyed();

	if(fighter
		&& ((fighter->Commands().Has(Command::DEPLOY) && !Random::Int(40 + 20 * !fighter->Attributes().Get("automaton")))
			|| (ejecting && !Random::Int(6))))
	{
		// Resupply any ships launching of their own accord.
		if(!ejecting)
		{
			// Determine which of the fighter's weapons we can restock.
			auto restockable = fighter->GetArmament().RestockableAmmo();
			auto toRestock = map<const Outfit *, int>{};
			for(auto &&ammo : restockable)
			{
				int count = fighter->OutfitCount(ammo);
				if(count > 0)
					toRestock.emplace(ammo, count);
			}
			auto takenAmmo = TransferAmmo(toRestock, *carrier, *fighter);
			bool tookAmmo = !takenAmmo.empty();
			if(tookAmmo)
			{
				// Update the carried mass cache.
				for(auto &&item : takenAmmo)
					carrier->AddCarriedMass(item.first->Mass() * item.second);
			}

			// This ship will refuel naturally based on the carrier's fuel
			// collection, but the carrier may have some reserves to spare.
			double maxFuel = fighter->Attributes().Get("fuel capacity");
			if(maxFuel)
			{
				double spareFuel = carrier->Fuel() - carrier->JumpNavigation().JumpFuel();
				if(spareFuel > 0.)
					carrier->TransferFuel(spareFuel, fighter.get());
				// If still low or out-of-fuel, re-stock the carrier and don't
				// launch, except if some ammo was taken (since we can fight).
				if(!tookAmmo && fighter->Fuel() < .25 * maxFuel)
				{
					fighter->TransferFuel(fighter->Fuel(), carrier.get());
				}
			}
		}
		// Those being ejected may be destroyed if they are already injured.
		else if(fighter->Health() < Random::Real())
			fighter->SelfDestruct();

		double maxV = fighter->MaxVelocity() * (1 + fighter->IsDestroyed());
		Point exitPoint = position + carrier->Facing().Rotate(position);
		// When ejected, ships depart haphazardly.
		Angle launchAngle = ejecting ? Angle(exitPoint - position) : carrier->Facing() + facing;
		Point v = carrier->Velocity() + (.3 * maxV) * launchAngle.Unit() + (.2 * maxV) * Angle::Random().Unit();
		fighter->Place(exitPoint, v, launchAngle, false);
		fighter->SetSystem(carrier->CurrentSystem());
		fighter->UnmarkForRemoval();
		// Update the cached sum of carried ship masses.
		carrier->AddCarriedMass(-fighter->Mass());
		// Create the desired launch effects.
		for(const Effect *effect : launchEffects)
			visuals.emplace_back(*effect, exitPoint, carrier->Velocity(), launchAngle);

		fighter.reset();
	}
}


void Bay::SetPosition(const Point &position)
{
	this->position = position;
}



void Bay::SetCategory(const std::string category)
{
	this->category = category;
}



void Bay::SetSide(const uint8_t side)
{
	this->side = side;
}



void Bay::SetFacing(const Angle &facing)
{
	this->facing = facing;
}



void Bay::SetCarrier(const std::shared_ptr<Ship> &ship)
{
	if(!ship)
		return;
	carrier = ship;
}



bool Bay::PositionFighter() const
{
	bool hasVisible = false;
	if(fighter && side)
	{
		hasVisible = true;
		fighter->SetPosition(Angle().Rotate(Position()) * carrier->Zoom() + fighter->Position());
		fighter->SetVelocity(carrier->Velocity());
		fighter->SetAngle(Angle() + Facing());
		fighter->SetZoom(carrier->Zoom());
	}
	return hasVisible;
}



bool Bay::CanCarry(const Ship &fighter) const
{
	// Check only for the category that we are interested in.
	const string &category = fighter.Attributes().Category();
	if(!fighter.shared_from_this())
		return true;
	if(fighter.shared_from_this() == this->fighter->shared_from_this())
		return true;
	if(fighter.Attributes().Category() == category && !fighter.IsDestroyed() &&
		(!carrier->IsYours() || (carrier->IsYours() && fighter.IsYours())))
		return false;
}



bool Bay::Carry(const shared_ptr <Ship> &fighter)
{
	if(!fighter || !fighter->CanBeCarried() || fighter->IsDisabled())
		return false;

	// Check only for the category that we are interested in.
	const string &category = fighter->Attributes().Category();

	// NPC ships should always transfer cargo. Player ships should only
	// transfer cargo if they set the AI preference.
	const bool shouldTransferCargo = Preferences::Has("Fighters transfer cargo");

	if((this->category == category) && !fighter)
	{
		this->fighter = fighter;
		fighter->SetSystem(nullptr);
		fighter->SetPlanet(nullptr);
		fighter->SetTargetSystem(nullptr);
		fighter->SetTargetStellar(nullptr);
		//fighter->isThrusting = false;
		//fighter->isReversing = false;
		//fighter->isSteering = false;
		//fighter->Commands().Clear();

		this->carrier = carrier;
		// If this fighter collected anything in space, try to store it.
		if(shouldTransferCargo && carrier->Cargo().Free() && !fighter->Cargo().IsEmpty())
			carrier->Cargo().TransferAll(fighter->Cargo());

		// Return unused fuel and ammunition to the carrier, so they may
		// be used by the carrier or other fighters.
		fighter->TransferFuel(fighter->Fuel(), carrier.get());

		// Determine the ammunition the fighter can supply.
		auto restockable = fighter->GetArmament().RestockableAmmo();
		auto toRestock = map<const Outfit *, int>{};
		for(auto &&ammo : restockable)
		{
			int count = fighter->OutfitCount(ammo);
			if(count > 0)
				toRestock.emplace(ammo, count);
		}
		TransferAmmo(toRestock, *fighter, *carrier);

		// Update the cached mass of the mothership.
		carrier->AddCarriedMass(fighter->Mass());
	}
}



void Bay::Unload()
{
	if(fighter)
	{
		carrier->AddCarriedMass(-fighter->Mass());
		fighter->SetSystem(carrier->CurrentSystem());
		fighter->SetPlanet(carrier->GetPlanet());
		fighter->UnmarkForRemoval();
		fighter.reset();
	}
}



map<const Outfit *, int> Bay::TransferAmmo(const map<const Outfit *, int> &stockpile, Ship &from, Ship &to)
{
	auto transferred = map<const Outfit *, int>{};
	for(auto &&item : stockpile)
	{
		assert(item.second > 0 && "stockpile count must be positive");
		int unloadable = abs(from.Attributes().CanAdd(*item.first, -item.second));
		int loadable = to.Attributes().CanAdd(*item.first, unloadable);
		if(loadable > 0)
		{
			from.AddOutfit(item.first, -loadable);
			to.AddOutfit(item.first, loadable);
			transferred[item.first] = loadable;
		}
	}
	return transferred;
}



void Bay::Deploy(bool includingDamaged) const
{
	if(Fighter() && (includingDamaged || Fighter()->Health() > .75) &&
		(!Fighter()->IsYours() || Fighter()->HasDeployOrder()))
		Fighter()->SetCommands(Command::DEPLOY);
}



bool Bay::HasDeployOrders() const
{
	if(Fighter() && Fighter()->HasDeployOrder())
		return true;
	else
		return false;
}



void Bay::Receive(const Broker::Message *message, const string &topic)
{
	if(topic == "bay")
	{
		Ship *ship = (Ship *) message->Publisher();
		if(ship->UUID() == carrier->UUID())
		{
			// Deploy
			if(message->Text() == "deploy")
				Deploy(false);
			if(message->Text() == "deploy including damaged")
				Deploy(true);

			// Has Deployments
			if(message->Text() == "has deploy orders")
				HasDeployOrders();
		}
	}
}
